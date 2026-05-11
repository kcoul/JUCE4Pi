"""
create_deck.py — builds the ADCJapan26 presentation from the QNX .potx template.
Add a new function per slide, call it in main().
Run: python create_deck.py
"""
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN, MSO_ANCHOR
from pptx.enum.shapes import MSO_CONNECTOR_TYPE
import io, os, zipfile

# ── paths ────────────────────────────────────────────────────────────────────
SLIDES_DIR = os.path.dirname(os.path.abspath(__file__))
TEMPLATE   = os.path.join(SLIDES_DIR, "QNX_PPT_Master_V11.potx")
OUTPUT     = os.path.join(SLIDES_DIR, "TXRX Part 2 - AI Audio on the Raspberry Pi.pptx")

# ── QNX brand palette ─────────────────────────────────────────────────────────
RED    = RGBColor(0xFF, 0x44, 0x3A)   # QNX primary red
RED2   = RGBColor(0x85, 0x35, 0x2E)   # deep red (destination nodes)
GRAY1  = RGBColor(0xC0, 0xC0, 0xBB)   # light gray (arrows, labels)
GRAY3  = RGBColor(0x5E, 0x5D, 0x5A)   # mid gray (host nodes)
PURPLE = RGBColor(0x8C, 0x62, 0xFF)   # energizer purple (boundary marker)
WHITE  = RGBColor(0xFF, 0xFF, 0xFF)

# ── MSO shape constants (integer form for portability) ────────────────────────
ROUNDED_RECTANGLE = 5
RIGHT_ARROW       = 13

# ── template helpers ──────────────────────────────────────────────────────────
def open_template(path):
    """Open a .potx by patching its content-type in memory."""
    buf = io.BytesIO()
    with zipfile.ZipFile(path, "r") as zin, \
         zipfile.ZipFile(buf, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "[Content_Types].xml":
                data = data.replace(
                    b"presentationml.template.main+xml",
                    b"presentationml.presentation.main+xml",
                )
            zout.writestr(item, data)
    buf.seek(0)
    return Presentation(buf)


def strip_template_slides(prs):
    """Remove all pre-existing sample slides baked into the template."""
    id_lst = prs.slides._sldIdLst
    rId_attr = "{http://schemas.openxmlformats.org/officeDocument/2006/relationships}id"
    for sld_id in list(id_lst):
        prs.part.drop_rel(sld_id.get(rId_attr))
        id_lst.remove(sld_id)


def get_layout(prs, master_idx, name):
    for lyt in prs.slide_masters[master_idx].slide_layouts:
        if lyt.name == name:
            return lyt
    raise ValueError(f"Layout '{name}' not found in master {master_idx}")


def set_ph(slide, idx, text):
    for ph in slide.placeholders:
        if ph.placeholder_format.idx == idx:
            ph.text = text
            return


# ── drawing helpers ───────────────────────────────────────────────────────────
def add_node(slide, x, y, w, h, lines, fill, text_color=None, font_size=9.5):
    """Rounded rectangle with centred multi-line text."""
    text_color = text_color or WHITE
    shape = slide.shapes.add_shape(
        ROUNDED_RECTANGLE, Inches(x), Inches(y), Inches(w), Inches(h)
    )
    shape.fill.solid()
    shape.fill.fore_color.rgb = fill
    shape.line.fill.background()

    tf = shape.text_frame
    tf.word_wrap = True
    tf.vertical_anchor = MSO_ANCHOR.MIDDLE
    for i, line in enumerate(lines):
        p = tf.paragraphs[0] if i == 0 else tf.add_paragraph()
        p.alignment = PP_ALIGN.CENTER
        run = p.add_run()
        run.text = line
        run.font.name = "Onest"
        run.font.size = Pt(font_size)
        run.font.color.rgb = text_color
    return shape


def add_arrow(slide, x, y, w, h, fill):
    """Right-pointing arrow shape."""
    shape = slide.shapes.add_shape(
        RIGHT_ARROW, Inches(x), Inches(y), Inches(w), Inches(h)
    )
    shape.fill.solid()
    shape.fill.fore_color.rgb = fill
    shape.line.fill.background()
    return shape


def add_vline(slide, x, y_top, y_bot, color, width_pt=1.5):
    """Vertical straight connector."""
    conn = slide.shapes.add_connector(
        MSO_CONNECTOR_TYPE.STRAIGHT,
        Inches(x), Inches(y_top), Inches(x), Inches(y_bot),
    )
    conn.line.color.rgb = color
    conn.line.width = Pt(width_pt)
    return conn


def add_label(slide, x, y, w, h, text, color, font_size=8,
              align=PP_ALIGN.CENTER, bold=False):
    """Transparent text label."""
    txb = slide.shapes.add_textbox(Inches(x), Inches(y), Inches(w), Inches(h))
    tf  = txb.text_frame
    tf.word_wrap = False
    p   = tf.paragraphs[0]
    p.alignment = align
    run = p.add_run()
    run.text = text
    run.font.name   = "Onest"
    run.font.size   = Pt(font_size)
    run.font.bold   = bold
    run.font.color.rgb = color
    return txb


# ── slides ────────────────────────────────────────────────────────────────────
def add_title_slide(prs):
    lyt   = get_layout(prs, 1, "Title | Quadrant Bar")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0,  "TX/RX Part 2:\nAI Audio on the Raspberry Pi")
    set_ph(slide, 1,  "Audio Developer Conference Japan 2026")
    set_ph(slide, 10, "Kieran Coulter | Principal Systems Software Developer - Audio")
    return slide


def add_signal_chain_slide(prs):
    """
    Full system signal chain:
      Voice -> Whisper+NPU -> Prog+Bank -> [boundary] -> Bridge -> SurgeXT -> Audio
    HOST nodes on the left, TARGET nodes on the right, separated by a purple line.
    """
    lyt   = get_layout(prs, 1, "Only Title")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "How It All Fits Together")

    # ── geometry (all in inches) ──────────────────────────────────────
    SLIDE_W = 13.33
    NW, NH  = 1.65, 0.82   # node width, height
    AW, AH  = 0.42, 0.22   # arrow width, height
    GAP     = 0.50          # total space between adjacent nodes
    Y_NODE  = 3.05          # node top edge
    Y_ARR   = Y_NODE + (NH - AH) / 2   # arrow vertically centred with node

    total_w = 6 * NW + 5 * GAP
    X0      = (SLIDE_W - total_w) / 2   # centre the row

    # left edge of each node / arrow
    node_x  = [X0 + i * (NW + GAP)               for i in range(6)]
    arrow_x = [node_x[i] + NW + (GAP - AW) / 2   for i in range(5)]

    # ── nodes ─────────────────────────────────────────────────────────
    # (text lines, fill colour)
    # Nodes 0-2: HOST  |  Nodes 3-5: TARGET
    spec = [
        (["Voice",    "Input"],          GRAY3),  # 0 — host
        (["Whisper",  "Hailo NPU"],      RED),    # 1 — hero: the NPU
        (["Prog +",   "Bank Change"],    GRAY3),  # 2 — host
        (["MIDI > OSC", "Bridge"],       GRAY3),  # 3 — target
        (["SurgeXT",  "on QNX"],         RED2),   # 4 — target: the synth
        (["Audio",    "Output"],         GRAY3),  # 5 — target
    ]
    for i, (lines, fill) in enumerate(spec):
        add_node(slide, node_x[i], Y_NODE, NW, NH, lines, fill)

    # ── arrows ────────────────────────────────────────────────────────
    for x in arrow_x:
        add_arrow(slide, x, Y_ARR, AW, AH, GRAY1)

    # ── HOST / TARGET boundary ────────────────────────────────────────
    # Gap between node 2 (last HOST) and node 3 (first TARGET)
    x_boundary = node_x[2] + NW + GAP / 2
    add_vline(slide, x_boundary, Y_NODE - 0.52, Y_NODE + NH + 0.52, PURPLE)

    # Zone labels
    host_w = x_boundary - X0
    tgt_w  = total_w - host_w
    add_label(slide, X0,                  Y_NODE - 0.52, host_w, 0.30,
              "HOST",                GRAY1, font_size=8, bold=True)
    add_label(slide, x_boundary + 0.05,   Y_NODE - 0.52, tgt_w,  0.30,
              "TARGET  (QNX / RPi5)", GRAY1, font_size=8, bold=True)

    return slide


def _cell_text(cell, text, color, font_size, font_name="Onest", align=PP_ALIGN.LEFT):
    """Set cell text with explicit font/colour, clearing any placeholder runs."""
    tf = cell.text_frame
    tf.vertical_anchor = MSO_ANCHOR.MIDDLE
    p = tf.paragraphs[0]
    p.alignment = align
    p.clear()           # remove any existing runs from the layout default
    run = p.add_run()
    run.text = text
    run.font.name  = font_name
    run.font.size  = Pt(font_size)
    run.font.color.rgb = color


def _style_table_borders(table):
    """
    Suppress all vertical borders; draw thin gray horizontal rules
    between rows only (lnB on every non-last row).
    """
    from pptx.oxml.ns import qn
    import lxml.etree as etree

    LINE_COLOR = "8F8F8B"   # GRAY2 — visible but subtle on dark background
    LINE_W     = "9525"     # 0.75 pt in EMU

    nrows = len(table.rows)

    for r, row in enumerate(table.rows):
        for cell in row.cells:
            tc   = cell._tc
            tcPr = tc.find(qn("a:tcPr"))
            if tcPr is None:
                tcPr = etree.SubElement(tc, qn("a:tcPr"))

            for side in ("a:lnL", "a:lnR", "a:lnT", "a:lnB"):
                el = tcPr.find(qn(side))
                if el is None:
                    el = etree.SubElement(tcPr, qn(side))
                for child in list(el):
                    el.remove(child)

                if side == "a:lnB" and r < nrows - 1:
                    el.set("w", LINE_W)
                    fill = etree.SubElement(el, qn("a:solidFill"))
                    etree.SubElement(fill, qn("a:srgbClr")).set("val", LINE_COLOR)
                else:
                    el.set("w", "0")
                    etree.SubElement(el, qn("a:noFill"))


def add_agenda_slide(prs):
    """
    Agenda as a borderless 6x2 table on 'Only Title' canvas.
    Col 0: "Chapter 0X." in QNX red, right-aligned (narrow)
    Col 1: chapter name in white Onest SemiBold, left-aligned (wide)
    No fills — dark master background shows through.
    Time allotments live in presentation_outline.md, not on this slide.
    """
    lyt   = get_layout(prs, 1, "Only Title")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "Agenda")

    ROWS = 6
    T_LEFT = Inches(1.20)
    T_TOP  = Inches(1.55)
    T_W    = Inches(10.93)
    T_H    = Inches(5.30)
    COL0_W = Inches(2.30)
    COL1_W = Inches(8.63)

    tbl = slide.shapes.add_table(ROWS, 2, T_LEFT, T_TOP, T_W, T_H).table
    tbl.columns[0].width = COL0_W
    tbl.columns[1].width = COL1_W

    # Disable header-row styling so every row renders identically
    tbl.first_row = False

    chapters = [
        ("Section 1.", "The Story So Far"),
        ("Section 2.", "JUCE on QNX"),
        ("Section 3.", "Audio on the NPU"),
        ("Section 4.", "Developing Host-First"),
        ("Section 5.", "Getting to the Target"),
        ("Section 6.", "Lessons & Next Steps"),
    ]

    for r, (num, name) in enumerate(chapters):
        c0 = tbl.cell(r, 0)
        c0.fill.background()
        c0.margin_right = Inches(0.25)   # breathing room between columns
        _cell_text(c0, num,  RED,   font_size=13, align=PP_ALIGN.RIGHT)

        c1 = tbl.cell(r, 1)
        c1.fill.background()
        _cell_text(c1, name, WHITE, font_size=20, font_name="Onest SemiBold")

    _style_table_borders(tbl)
    return slide


def add_divider(prs, chapter_num, title):
    """Dark-theme chapter divider slide (slideLayout29 = 'Divider', master[1])."""
    lyt   = get_layout(prs, 1, "Divider")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0,  title)
    set_ph(slide, 13, f"Section {chapter_num}.")
    return slide


def add_quadrant_slide(prs, title, center_label, quadrants):
    """
    Recreates the Slide Library slide-44 'status report' layout:
      - thin cross lines dividing the content area into four quadrants
      - a filled rounded-square badge at the intersection (center_label)
      - four text areas, each with a small GRAY1 header + larger WHITE body
    quadrants = [(header, body), ...] in order: top-left, top-right,
                                                bottom-left, bottom-right
    """
    lyt   = get_layout(prs, 1, "Only Title")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, title)

    # ── geometry ─────────────────────────────────────────────────────
    MX, MY   = 1.00, 1.55   # content area top-left margin
    EX, EY   = 12.33, 6.75  # content area bottom-right edge
    MID_X    = (MX + EX) / 2        # 6.665"
    MID_Y    = (MY + EY) / 2        # 4.15" → nudge down slightly
    MID_Y    = 4.05
    BOX      = 1.65                  # center badge side length
    PAD      = 0.20                  # gap between divider line and text

    # ── cross lines (drawn first so badge sits on top) ────────────────
    for (x1, y1, x2, y2) in [
        (MX, MID_Y, EX, MID_Y),          # horizontal
        (MID_X, MY,  MID_X, EY),         # vertical
    ]:
        conn = slide.shapes.add_connector(
            MSO_CONNECTOR_TYPE.STRAIGHT,
            Inches(x1), Inches(y1), Inches(x2), Inches(y2),
        )
        conn.line.color.rgb = GRAY3
        conn.line.width = Pt(0.75)

    # ── center badge ──────────────────────────────────────────────────
    bx, by = MID_X - BOX / 2, MID_Y - BOX / 2
    badge = slide.shapes.add_shape(
        ROUNDED_RECTANGLE, Inches(bx), Inches(by), Inches(BOX), Inches(BOX),
    )
    badge.fill.solid()
    badge.fill.fore_color.rgb = RED
    badge.line.fill.background()
    tf = badge.text_frame
    tf.vertical_anchor = MSO_ANCHOR.MIDDLE
    p = tf.paragraphs[0]
    p.alignment = PP_ALIGN.CENTER
    run = p.add_run()
    run.text = center_label
    run.font.name  = "Onest SemiBold"
    run.font.size  = Pt(20)
    run.font.color.rgb = WHITE

    # ── quadrant text areas ───────────────────────────────────────────
    wL = MID_X - MX - PAD         # width of left quadrants
    wR = EX - MID_X - PAD         # width of right quadrants
    hT = MID_Y - MY - PAD         # height of top quadrants
    hB = EY - MID_Y - PAD         # height of bottom quadrants

    positions = [
        (MX,          MY,          wL, hT),   # top-left
        (MID_X + PAD, MY,          wR, hT),   # top-right
        (MX,          MID_Y + PAD, wL, hB),   # bottom-left
        (MID_X + PAD, MID_Y + PAD, wR, hB),   # bottom-right
    ]

    for (x, y, w, h), (header, body) in zip(positions, quadrants):
        txb = slide.shapes.add_textbox(Inches(x), Inches(y), Inches(w), Inches(h))
        tf  = txb.text_frame
        tf.word_wrap = True
        tf.vertical_anchor = MSO_ANCHOR.MIDDLE

        p = tf.paragraphs[0]
        p.alignment = PP_ALIGN.LEFT
        run = p.add_run()
        run.text = header
        run.font.name  = "Onest"
        run.font.size  = Pt(10)
        run.font.color.rgb = GRAY1

        p2 = tf.add_paragraph()
        p2.alignment = PP_ALIGN.LEFT
        run2 = p2.add_run()
        run2.text = body
        run2.font.name  = "Onest SemiBold"
        run2.font.size  = Pt(14)
        run2.font.color.rgb = WHITE

    return slide


# ── Section 2 slides ─────────────────────────────────────────────────────────

def add_qnx_everywhere_slide(prs):
    """S2.2 — QNX Everywhere initiative: JUCE as a natural porting target."""
    lyt   = get_layout(prs, 1, "Only Title")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "QNX Everywhere")
    # TODO: content
    return slide


def _add_fullscreen_timing(slide, sp_id):
    """
    Inject a <p:timing> tree that plays shape sp_id fullscreen on click.
    PowerPoint's 'Play Full Screen' checkbox writes exactly this XML;
    python-pptx never generates it automatically, so we push it via lxml.
    """
    import lxml.etree as etree

    NS_P = "http://schemas.openxmlformats.org/presentationml/2006/main"
    NS_A = "http://schemas.openxmlformats.org/drawingml/2006/main"

    xml = f"""\
<p:timing xmlns:p="{NS_P}" xmlns:a="{NS_A}">
  <p:tnLst>
    <p:par>
      <p:cTn id="1" dur="indefin" restart="whenNotActive" nodeType="tmRoot">
        <p:childTnLst>
          <p:seq concurrent="1" nextAc="seek">
            <p:cTn id="2" dur="indefin" nodeType="mainSeq">
              <p:childTnLst>
                <p:par>
                  <p:cTn id="3" fill="hold">
                    <p:stCondLst>
                      <p:cond delay="indefin"/>
                    </p:stCondLst>
                    <p:childTnLst>
                      <p:par>
                        <p:cTn id="4" fill="hold">
                          <p:stCondLst>
                            <p:cond delay="0"/>
                          </p:stCondLst>
                          <p:childTnLst>
                            <p:video fullScrn="1">
                              <p:cMediaNode vol="80000">
                                <p:cTn id="5" dur="indefin" fill="hold"/>
                                <p:tgtEl>
                                  <p:spTgt spid="{sp_id}"/>
                                </p:tgtEl>
                              </p:cMediaNode>
                            </p:video>
                          </p:childTnLst>
                        </p:cTn>
                      </p:par>
                    </p:childTnLst>
                  </p:cTn>
                </p:par>
              </p:childTnLst>
            </p:cTn>
            <p:prevCondLst>
              <p:cond evt="onPrevClick" delay="0">
                <p:tn/>
              </p:cond>
            </p:prevCondLst>
          </p:seq>
        </p:childTnLst>
      </p:cTn>
    </p:par>
  </p:tnLst>
  <p:bldLst/>
</p:timing>"""

    slide._element.append(etree.fromstring(xml.encode()))


def add_video_slide(prs, video_path, title, poster_path=None):
    """
    Embed a video centred in the content area with an optional poster frame.
    If video_path doesn't exist, renders a branded placeholder box instead.
    Video is sized at ~70% of the slide width (maintains 16:9).
    Encoding requirements: H.264 baseline/main, yuv420p, AAC, -movflags +faststart.
    """
    lyt   = get_layout(prs, 1, "Only Title")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, title)

    # ── video geometry (70% width, 16:9, centred in content area) ────
    VW = Inches(9.0)
    VH = Inches(VW / Inches(1) * (9 / 16))   # 16:9 → 5.0625"
    VH = Inches(5.06)
    slide_w, content_top = Inches(13.33), Inches(1.45)
    content_h = Inches(7.5) - content_top - Inches(0.3)
    VX = (slide_w - VW) / 2
    VY = content_top + (content_h - VH) / 2

    if os.path.exists(video_path):
        movie = slide.shapes.add_movie(
            video_path, VX, VY, VW, VH,
            poster_frame_image=poster_path,
            mime_type="video/mp4",
        )
        # _add_fullscreen_timing(slide, movie.shape_id)
        # ^ Disabled: PowerPoint rejects the timing XML we inject.
        #   Enable "Play Full Screen" manually via Video Format tab after opening.
    else:
        # Stand-in box until the real video is dropped in
        box = slide.shapes.add_shape(
            ROUNDED_RECTANGLE, VX, VY, VW, VH,
        )
        box.fill.solid()
        box.fill.fore_color.rgb = GRAY3
        box.line.color.rgb      = GRAY1
        box.line.width          = Pt(1)
        tf = box.text_frame
        tf.vertical_anchor = MSO_ANCHOR.MIDDLE
        p  = tf.paragraphs[0]
        p.alignment = PP_ALIGN.CENTER
        run = p.add_run()
        run.text = f"[ VIDEO : {os.path.basename(video_path)} ]"
        run.font.name  = "Onest"
        run.font.size  = Pt(14)
        run.font.color.rgb = GRAY1

    return slide


def add_juce_porting_lessons_slide(prs):
    """S2.4 — Key technical lessons from porting JUCE to QNX."""
    lyt   = get_layout(prs, 1, "Only Title")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "Lessons from the Port")
    # TODO: content — challenges/surprises from JUCE QNX port
    return slide


def add_tracktion_engine_slide(prs):
    """S2.5 — Tracktion Engine also ported: capstone of Section 2."""
    lyt   = get_layout(prs, 1, "Only Title")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "And Tracktion Engine Too")
    # TODO: content
    return slide


# ── Section 2 slides ─────────────────────────────────────────────────────────

def add_signal_chain_full_target_slide(prs):
    """
    Same six nodes as add_signal_chain_slide, but ALL running on the target.
    No HOST/TARGET split — the boundary line is gone.
    This is the "I would have thought this impossible five years ago" slide.
    """
    lyt   = get_layout(prs, 1, "Only Title")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "The Full System on Target")

    # ── geometry (identical to signal chain slide) ────────────────────
    SLIDE_W = 13.33
    NW, NH  = 1.65, 0.82
    AW, AH  = 0.42, 0.22
    GAP     = 0.50
    Y_NODE  = 3.05
    Y_ARR   = Y_NODE + (NH - AH) / 2

    total_w = 6 * NW + 5 * GAP
    X0      = (SLIDE_W - total_w) / 2

    node_x  = [X0 + i * (NW + GAP)               for i in range(6)]
    arrow_x = [node_x[i] + NW + (GAP - AW) / 2   for i in range(5)]

    # ── nodes — same colours as diagram 1 ────────────────────────────
    spec = [
        (["Voice",      "Input"],        GRAY3),
        (["Whisper",    "Hailo NPU"],    RED),    # hero — NPU on-device
        (["Prog +",     "Bank Change"],  GRAY3),
        (["MIDI > OSC", "Bridge"],       GRAY3),
        (["SurgeXT",    "on QNX"],       RED2),
        (["Audio",      "Output"],       GRAY3),
    ]
    for i, (lines, fill) in enumerate(spec):
        add_node(slide, node_x[i], Y_NODE, NW, NH, lines, fill)

    # ── arrows ────────────────────────────────────────────────────────
    for x in arrow_x:
        add_arrow(slide, x, Y_ARR, AW, AH, GRAY1)

    # ── single TARGET label spanning all six nodes ────────────────────
    add_label(slide, X0, Y_NODE - 0.52, total_w, 0.30,
              "TARGET  (QNX / RPi5)", GRAY1, font_size=8, bold=True)

    return slide


# ── main ──────────────────────────────────────────────────────────────────────
def main():
    prs = open_template(TEMPLATE)
    strip_template_slides(prs)

    # ── title + agenda ────────────────────────────────────────────
    add_title_slide(prs)
    add_agenda_slide(prs)

    # ── Section 1: The Story So Far ──────────────────────────────
    add_divider(prs, 1, "The Story So Far")

    add_quadrant_slide(
        prs,
        title="Recap",
        center_label="ADC'21",
        quadrants=[
            ("Audience",
             "Beginner-friendly — no embedded or cross-compilation experience required"),
            ("Hardware",
             "Raspberry Pi 4 · Raspberry Pi OS · JUCE apps and plugins compiled on-device"),
            ("The Trick",
             "Mount the RPi filesystem over SSH — skip cross-compilation entirely"),
            ("The Lesson",
             "Get something running first. Optimise the dev workflow when you actually need to"),
        ],
    )

    add_quadrant_slide(
        prs,
        title="Recap",
        center_label="ADCx'23",
        quadrants=[
            ("The Project",
             "NeuralPlayer — a host-side pipeline combining Stem Separation and Audio-to-MIDI conversion models"),
            ("The Philosophy",
             "Pure R&D belongs on the host: no target hardware, no cross-compilation, no embedded constraints"),
            ("The Lesson",
             "The further towards Research on the R&D spectrum, the more premature target work costs in friction"),
            ("Coming in Part 4",
             "NeuralPlayer didn't stop here — a future talk will reveal where this project went next"),
        ],
    )

    # ── Section 2: JUCE on QNX ───────────────────────────────────
    add_divider(prs, 2, "JUCE on QNX")
    add_qnx_everywhere_slide(prs)
    add_video_slide(
        prs,
        video_path=os.path.join(SLIDES_DIR, "placeholder.mp4"),
        title="It Works.",
    )
    add_juce_porting_lessons_slide(prs)
    add_tracktion_engine_slide(prs)

    # ── Section 3: Audio on the NPU ──────────────────────────────
    add_divider(prs, 3, "Audio on the NPU")
    # TODO: content

    # ── Section 4: Developing Host-First ─────────────────────────
    add_divider(prs, 4, "Developing Host-First")
    # TODO: content

    # ── Section 5: Getting to the Target ─────────────────────────
    add_divider(prs, 5, "Getting to the Target")
    add_signal_chain_slide(prs)
    add_signal_chain_full_target_slide(prs)
    # TODO: remaining content

    # ── Section 6: Lessons & Next Steps ──────────────────────────
    add_divider(prs, 6, "Lessons & Next Steps")
    # TODO: content

    prs.save(OUTPUT)
    print(f"Saved {len(prs.slides)} slides: {OUTPUT}")


if __name__ == "__main__":
    main()
