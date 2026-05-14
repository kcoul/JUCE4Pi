#!/usr/bin/env bash
set -euo pipefail

# Classify submodules as active/dormant by comparing origin owner/namespace
# against the root repo origin owner/namespace.
#
# Active: same owner/namespace as root repo (and same host)
# Dormant: different owner/namespace
#
# Usage:
#   scripts/manage-active-submodules.sh
#   scripts/manage-active-submodules.sh --apply
#
# Options:
#   --apply                 Attach active submodules to a branch and install a detached-HEAD pre-commit guard
#   --default-branch <name> Fallback branch name when origin/HEAD is unavailable (default: main)

apply=false
default_branch="main"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --apply) apply=true; shift ;;
    --default-branch)
      default_branch="${2:-}"
      if [[ -z "$default_branch" ]]; then
        echo "Missing value for --default-branch" >&2
        exit 2
      fi
      shift 2
      ;;
    *)
      echo "Unknown argument: $1" >&2
      exit 2
      ;;
  esac
done

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

if [[ ! -f .gitmodules ]]; then
  echo "No .gitmodules found in $repo_root"
  exit 0
fi

extract_host_and_owner() {
  local url="$1"
  local host=""
  local owner=""

  # ssh: git@github.com:owner/repo.git
  if [[ "$url" =~ ^[^@]+@([^:]+):([^/]+)/.+$ ]]; then
    host="${BASH_REMATCH[1]}"
    owner="${BASH_REMATCH[2]}"
  # https/http/ssh-url style: https://host/owner/repo(.git)
  elif [[ "$url" =~ ^[a-zA-Z][a-zA-Z0-9+.-]*://([^/]+)/([^/]+)/.+$ ]]; then
    host="${BASH_REMATCH[1]}"
    owner="${BASH_REMATCH[2]}"
  # scp-like without user: host:owner/repo.git
  elif [[ "$url" =~ ^([^:]+):([^/]+)/.+$ ]]; then
    host="${BASH_REMATCH[1]}"
    owner="${BASH_REMATCH[2]}"
  fi

  printf '%s\t%s\n' "$host" "$owner"
}

root_url="$(git remote get-url origin)"
read -r root_host root_owner < <(extract_host_and_owner "$root_url")

if [[ -z "$root_host" || -z "$root_owner" ]]; then
  echo "Could not parse root origin URL: $root_url" >&2
  exit 1
fi

echo "Root origin: $root_url"
echo "Root host/owner: $root_host / $root_owner"
echo
printf '%-10s  %-45s  %-18s  %s\n' "Class" "Submodule" "Owner" "URL"
printf '%-10s  %-45s  %-18s  %s\n' "----------" "---------------------------------------------" "------------------" "------------------------------"

active_paths=()

while IFS= read -r sm_path; do
  [[ -z "$sm_path" ]] && continue

  if [[ ! -d "$sm_path/.git" && ! -f "$sm_path/.git" ]]; then
    printf '%-10s  %-45s  %-18s  %s\n' "missing" "$sm_path" "-" "(not initialized)"
    continue
  fi

  sm_url="$(git -C "$sm_path" remote get-url origin 2>/dev/null || true)"
  if [[ -z "$sm_url" ]]; then
    printf '%-10s  %-45s  %-18s  %s\n' "unknown" "$sm_path" "-" "(no origin)"
    continue
  fi

  read -r sm_host sm_owner < <(extract_host_and_owner "$sm_url")
  if [[ -z "$sm_host" || -z "$sm_owner" ]]; then
    printf '%-10s  %-45s  %-18s  %s\n' "unknown" "$sm_path" "-" "$sm_url"
    continue
  fi

  if [[ "$sm_host" == "$root_host" && "$sm_owner" == "$root_owner" ]]; then
    cls="active"
    active_paths+=("$sm_path")
  else
    cls="dormant"
  fi

  printf '%-10s  %-45s  %-18s  %s\n' "$cls" "$sm_path" "$sm_owner@$sm_host" "$sm_url"
done < <(git submodule status --recursive | awk '{print $2}')

if ! $apply; then
  echo
  echo "Dry run only. Re-run with --apply to configure active submodules."
  exit 0
fi

echo
for sm_path in "${active_paths[@]}"; do
  echo "[apply] $sm_path"

  current_branch="$(git -C "$sm_path" symbolic-ref -q --short HEAD || true)"

  # Determine target branch
  target_branch=""
  if [[ -n "$current_branch" ]]; then
    target_branch="$current_branch"
  else
    remote_head="$(git -C "$sm_path" symbolic-ref -q --short refs/remotes/origin/HEAD || true)"
    if [[ -n "$remote_head" ]]; then
      target_branch="${remote_head#origin/}"
    else
      target_branch="$default_branch"
    fi
  fi

  if [[ -n "$(git -C "$sm_path" status --porcelain)" ]]; then
    echo "  [warn] dirty tree, skipping branch attach"
  else
    if [[ "$current_branch" == "$target_branch" && -n "$current_branch" ]]; then
      echo "  [ok] already on branch $target_branch"
    else
      git -C "$sm_path" fetch --quiet origin "$target_branch" || true
      if git -C "$sm_path" show-ref --verify --quiet "refs/heads/$target_branch"; then
        git -C "$sm_path" switch "$target_branch" >/dev/null
        echo "  [fix] switched to local branch $target_branch"
      elif git -C "$sm_path" show-ref --verify --quiet "refs/remotes/origin/$target_branch"; then
        git -C "$sm_path" switch -c "$target_branch" --track "origin/$target_branch" >/dev/null
        echo "  [fix] created tracking branch $target_branch"
      else
        echo "  [warn] cannot find branch $target_branch locally or on origin"
      fi
    fi
  fi

  hooks_dir="$(git -C "$sm_path" rev-parse --git-path hooks)"
  mkdir -p "$hooks_dir"
  cat > "$hooks_dir/pre-commit" <<'HOOK'
#!/usr/bin/env bash
set -euo pipefail

if ! git symbolic-ref -q HEAD >/dev/null; then
  echo "Refusing commit: detached HEAD. Switch to a branch first." >&2
  exit 1
fi
HOOK
  chmod +x "$hooks_dir/pre-commit"
  echo "  [fix] installed detached-HEAD pre-commit guard"
done

echo
echo "Done."
