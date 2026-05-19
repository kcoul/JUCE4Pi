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
GREEN  = RGBColor(0x06, 0xCA, 0x5F)   # pros / gain (Slide-46 template)
REDCON = RGBColor(0xE4, 0x20, 0x3C)   # cons / loss (Slide-46 template)
GRAY1  = RGBColor(0xC0, 0xC0, 0xBB)   # light gray (arrows, labels)
GRAY3  = RGBColor(0x5E, 0x5D, 0x5A)   # mid gray (host nodes)
DARK1  = RGBColor(0x24, 0x24, 0x22)   # dark card fill
PURPLE = RGBColor(0x8C, 0x62, 0xFF)   # energizer purple (boundary marker)
WHITE  = RGBColor(0xFF, 0xFF, 0xFF)

# ── MSO shape constants (integer form for portability) ────────────────────────
ROUNDED_RECTANGLE = 5
RIGHT_ARROW       = 33

# ── template helpers ──────────────────────────────────────────────────────────
def _image_dims(path):
    """
    Return (width_px, height_px) for a JPEG or PNG without PIL.
    Parses JPEG SOF markers or PNG IHDR directly.
    Returns (None, None) if the format is unrecognised.
    """
    import struct
    with open(path, "rb") as f:
        header = f.read(24)
    # PNG — dimensions are at bytes 16-24 of the file header
    if header[:8] == b"\x89PNG\r\n\x1a\n":
        w, h = struct.unpack(">II", header[16:24])
        return w, h
    # JPEG — walk markers until SOF0/SOF1/SOF2
    if header[:2] == b"\xff\xd8":
        with open(path, "rb") as f:
            f.read(2)
            while True:
                marker = f.read(2)
                if len(marker) < 2 or marker[0] != 0xFF:
                    break
                m = marker[1]
                if m in (0xC0, 0xC1, 0xC2):   # SOF markers carry dimensions
                    f.read(3)                   # skip length + precision bytes
                    h, w = struct.unpack(">HH", f.read(4))
                    return w, h
                elif m == 0xD9:                # EOI — give up
                    break
                else:
                    length = struct.unpack(">H", f.read(2))[0]
                    f.read(length - 2)
    return None, None


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


def set_content_ph(slide, idx, header, items):
    """
    Populate a content/body placeholder using the template's native paragraph levels.

    Level 1 → 18 pt sub-headline, no bullet  (master lvl2pPr)  — column header
    Level 2 → 14 pt + red-square bullet       (master lvl3pPr)  — bullet items

    All font sizes, colours, and bullet characters inherit from the dark master.
    No explicit overrides — this function works WITH the template, not against it.
    Keep items to 4 max for back-of-room legibility at 14 pt.
    """
    for ph in slide.placeholders:
        if ph.placeholder_format.idx == idx:
            tf = ph.text_frame
            tf.word_wrap = True
            p = tf.paragraphs[0]
            p.clear()
            p.level = 1          # 18 pt sub-headline, no bullet
            run = p.add_run()
            run.text = header
            for item in items:
                p2 = tf.add_paragraph()
                p2.level = 2     # 14 pt + red-square bullet from master
                run2 = p2.add_run()
                run2.text = item
            return


# ── SVG image helper ──────────────────────────────────────────────────────────
def add_svg_image(slide, svg_path, left_in, top_in, width_in, height_in, fill_subs=None):
    """
    Embed an SVG on the slide using raw OOXML part injection.
    PowerPoint 2016+ renders it as vector; older viewers fall back to a 1×1
    transparent PNG so no broken-image icon appears.
    python-pptx has no native SVG API — we create the Part/relationship manually.
    """
    import uuid, base64
    import lxml.etree as etree
    from pptx.opc.package import Part
    from pptx.opc.packuri import PackURI

    IMG_REL = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image"
    NS_P   = "http://schemas.openxmlformats.org/presentationml/2006/main"
    NS_A   = "http://schemas.openxmlformats.org/drawingml/2006/main"
    NS_R   = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
    NS_SVG = "http://schemas.microsoft.com/office/drawing/2016/SVG/main"

    FALLBACK_PNG = base64.b64decode(
        "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhf"
        "DwAChwGA60e6kgAAAABJRU5ErkJggg=="
    )

    with open(svg_path, "rb") as f:
        svg_bytes = f.read()
    for old, new in (fill_subs or {}).items():
        svg_bytes = svg_bytes.replace(old, new)

    uid      = uuid.uuid4().hex[:8]
    sp       = slide.part
    pkg      = sp.package

    png_part = Part(PackURI(f"/ppt/media/svg_fb_{uid}.png"),
                   "image/png", pkg, FALLBACK_PNG)
    svg_part = Part(PackURI(f"/ppt/media/svg_{uid}.svg"),
                   "image/svg+xml", pkg, svg_bytes)

    rid_png = sp.relate_to(png_part, IMG_REL)
    rid_svg = sp.relate_to(svg_part, IMG_REL)

    x_emu  = int(left_in   * 914400)
    y_emu  = int(top_in    * 914400)
    cx_emu = int(width_in  * 914400)
    cy_emu = int(height_in * 914400)
    guid   = str(uuid.uuid4()).upper()

    pic_xml = (
        f'<p:pic xmlns:p="{NS_P}" xmlns:a="{NS_A}" xmlns:r="{NS_R}">'
        f'<p:nvPicPr>'
        f'<p:cNvPr id="200" name="SVGLogo">'
        f'<a:extLst><a:ext uri="{{FF2B5EF4-FFF2-40B4-BE49-F238E27FC236}}">'
        f'<a16:creationId xmlns:a16="{NS_SVG}" id="{{{guid}}}"/>'
        f'</a:ext></a:extLst></p:cNvPr>'
        f'<p:cNvPicPr><a:picLocks noChangeAspect="1"/></p:cNvPicPr>'
        f'<p:nvPr/></p:nvPicPr>'
        f'<p:blipFill>'
        f'<a:blip r:embed="{rid_png}">'
        f'<a:extLst><a:ext uri="{{96DAC541-7B7A-43D3-8B79-37D633B846F1}}">'
        f'<asvg:svgBlip xmlns:asvg="{NS_SVG}" r:embed="{rid_svg}"/>'
        f'</a:ext></a:extLst>'
        f'</a:blip>'
        f'<a:stretch><a:fillRect/></a:stretch>'
        f'</p:blipFill>'
        f'<p:spPr>'
        f'<a:xfrm><a:off x="{x_emu}" y="{y_emu}"/>'
        f'<a:ext cx="{cx_emu}" cy="{cy_emu}"/></a:xfrm>'
        f'<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
        f'<a:ln><a:noFill/></a:ln>'
        f'</p:spPr>'
        f'</p:pic>'
    )

    slide.shapes._spTree.append(etree.fromstring(pic_xml.encode()))


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
              align=PP_ALIGN.CENTER, bold=False, url=None):
    """Transparent text label. Pass url to make the text a clickable hyperlink."""
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
    if url:
        run.hyperlink.address = url
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

    ROWS = 5
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


def _add_support_card(slide, x, y, w, h, step, title, subtitle, body_lines, accent):
    """Slide-41-style process card: badge, title, short subtitle, and body lines."""
    card = slide.shapes.add_shape(
        ROUNDED_RECTANGLE, Inches(x), Inches(y), Inches(w), Inches(h)
    )
    card.fill.solid()
    card.fill.fore_color.rgb = DARK1
    card.line.color.rgb = GRAY3
    card.line.width = Pt(0.75)

    badge = slide.shapes.add_shape(
        ROUNDED_RECTANGLE, Inches(x + 0.12), Inches(y + 0.14), Inches(0.52), Inches(0.46)
    )
    badge.fill.solid()
    badge.fill.fore_color.rgb = accent
    badge.line.fill.background()
    tf = badge.text_frame
    tf.vertical_anchor = MSO_ANCHOR.MIDDLE
    p = tf.paragraphs[0]
    p.alignment = PP_ALIGN.CENTER
    run = p.add_run()
    run.text = str(step)
    run.font.name = "Onest SemiBold"
    run.font.size = Pt(14)
    run.font.color.rgb = WHITE

    add_label(slide, x + 0.72, y + 0.22, w - 0.84, 0.24,
              f"STEP {step}", GRAY1, font_size=7.5, align=PP_ALIGN.LEFT, bold=True)

    txb = slide.shapes.add_textbox(Inches(x + 0.16), Inches(y + 0.82),
                                   Inches(w - 0.32), Inches(h - 0.98))
    tf = txb.text_frame
    tf.word_wrap = True
    tf.margin_left = Inches(0.02)
    tf.margin_right = Inches(0.02)
    tf.margin_top = Inches(0.00)
    tf.margin_bottom = Inches(0.00)

    p = tf.paragraphs[0]
    p.alignment = PP_ALIGN.LEFT
    run = p.add_run()
    run.text = title
    run.font.name = "Onest SemiBold"
    run.font.size = Pt(15)
    run.font.color.rgb = WHITE

    p = tf.add_paragraph()
    p.alignment = PP_ALIGN.LEFT
    run = p.add_run()
    run.text = subtitle
    run.font.name = "Onest"
    run.font.size = Pt(9)
    run.font.color.rgb = accent

    for line in body_lines:
        p = tf.add_paragraph()
        p.alignment = PP_ALIGN.LEFT
        run = p.add_run()
        run.text = line
        run.font.name = "Onest"
        run.font.size = Pt(9)
        run.font.color.rgb = GRAY1

    return card


def add_host_target_support_matrix_slide(prs):
    """
    Adapted from QNX Slide Library slide 41: four process steps across the page.
    Shows the practical path for Windows-first development across Linux and QNX.
    """
    lyt   = get_layout(prs, 1, "Only Title")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "Host / Target Support Matrix")

    # Geometry mirrors the four-column process layout in QNX Slide Library slide 41.
    xs = [0.68, 3.83, 6.98, 10.13]
    y, w, h = 2.20, 2.52, 4.30

    host_w = xs[1] + w - xs[0]
    target_w = xs[3] + w - xs[2]
    add_label(slide, xs[0], 1.82, host_w, 0.26, "HOSTS", GRAY1, font_size=8, bold=True)
    add_label(slide, xs[2], 1.82, target_w, 0.26, "TARGETS", GRAY1, font_size=8, bold=True)

    boundary_x = (xs[1] + w + xs[2]) / 2
    add_vline(slide, boundary_x, 1.78, 6.72, PURPLE, width_pt=1.25)

    cards = [
        (1, "Windows Host", "Windows 11",
         ["Builds the bridge for Windows",
          "QNX SDP works natively",
          "Hailo backend stays disabled"], RED),
        (2, "Ubuntu Host", "Native Linux or WSL2",
         ["Uses apt multiarch packages",
          "Cross-compiles for Ubuntu arm64",
          "Links the HailoRT arm64 sysroot"], RED),
        (3, "Ubuntu Target", "Raspberry Pi 5",
         ["Runs the Linux arm64 bridge",
          "Exercises Hailo-10H over PCIe",
          "Validates the voice-command path"], RED),
        (4, "QNX Target", "Raspberry Pi 5",
         ["Runs SurgeXT on QNX",
          "Uses the QNX cross SDK",
          "Keeps the same host-first workflow"], RED),
    ]

    for x, card in zip(xs, cards):
        _add_support_card(slide, x, y, w, h, *card)

    for x in [3.24, 6.39, 9.55]:
        add_arrow(slide, x, 3.33, 0.55, 0.28, GRAY1)

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


# ── Section 3 helpers ────────────────────────────────────────────────────────

def _inject_arrow(slide, x_in, y_in, size_in, color_hex, rotate_180=False):
    """
    Inject a custom-geometry arrow shape (Slide-46 template style) via raw XML.
    The base path points DOWN; rotate_180=True flips it UP for the Pros row.
    """
    import lxml.etree as etree
    x_emu  = int(x_in    * 914400)
    y_emu  = int(y_in    * 914400)
    sz_emu = int(size_in * 914400)
    rot    = ' rot="10800000"' if rotate_180 else ""
    NS_P   = "http://schemas.openxmlformats.org/presentationml/2006/main"
    NS_A   = "http://schemas.openxmlformats.org/drawingml/2006/main"
    xml = (
        f'<p:sp xmlns:p="{NS_P}" xmlns:a="{NS_A}">'
        f'<p:nvSpPr><p:cNvPr id="0" name="Arrow"/>'
        f'<p:cNvSpPr><a:spLocks noChangeAspect="1"/></p:cNvSpPr>'
        f'<p:nvPr/></p:nvSpPr>'
        f'<p:spPr><a:xfrm{rot}>'
        f'<a:off x="{x_emu}" y="{y_emu}"/>'
        f'<a:ext cx="{sz_emu}" cy="{sz_emu}"/>'
        f'</a:xfrm>'
        f'<a:custGeom><a:avLst/><a:gdLst/><a:ahLst/><a:cxnLst/>'
        f'<a:rect l="l" t="t" r="r" b="b"/>'
        f'<a:pathLst><a:path w="152400" h="152400">'
        f'<a:moveTo><a:pt x="66675" y="0"/></a:moveTo>'
        f'<a:lnTo><a:pt x="66675" y="115967"/></a:lnTo>'
        f'<a:lnTo><a:pt x="13335" y="62627"/></a:lnTo>'
        f'<a:lnTo><a:pt x="0" y="76200"/></a:lnTo>'
        f'<a:lnTo><a:pt x="76200" y="152400"/></a:lnTo>'
        f'<a:lnTo><a:pt x="152400" y="76200"/></a:lnTo>'
        f'<a:lnTo><a:pt x="139065" y="62627"/></a:lnTo>'
        f'<a:lnTo><a:pt x="85725" y="115967"/></a:lnTo>'
        f'<a:lnTo><a:pt x="85725" y="0"/></a:lnTo>'
        f'<a:lnTo><a:pt x="66675" y="0"/></a:lnTo>'
        f'<a:close/>'
        f'</a:path></a:pathLst></a:custGeom>'
        f'<a:solidFill><a:srgbClr val="{color_hex}"/></a:solidFill>'
        f'<a:ln w="238" cap="flat"><a:noFill/></a:ln>'
        f'</p:spPr>'
        f'<p:txBody><a:bodyPr rtlCol="0" anchor="ctr"/><a:lstStyle/>'
        f'<a:p><a:endParaRPr/></a:p></p:txBody>'
        f'</p:sp>'
    )
    slide.shapes._spTree.append(etree.fromstring(xml.encode()))


def add_npu_pros_cons_slide(prs):
    """
    S3.1 — Audio on the NPU: Pros & Cons.
    Adapted from QNX Slide Library slide 46 (Gain & Loss layout):
      green outlined box + up-arrow  → Pros row (quoted source stat)
      red   outlined box + down-arrow → Cons row (placeholder)
    """
    lyt   = get_layout(prs, 1, "Only Title")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "Audio on the NPU — Pros & Cons")

    # ── geometry (mirrors Slide Library slide 46 exactly) ─────────
    BOX_X, BOX_W, BOX_H = 0.68, 1.89, 1.89
    ARR_W                = 0.87
    ARR_X                = BOX_X + (BOX_W - ARR_W) / 2   # 1.19" centred in box
    TXT_X, TXT_W, TXT_H = 3.20, 9.45, 2.13
    PRO_Y, CON_Y         = 1.94, 4.70
    DIV_Y                = 4.26

    # ── Pros: green outlined box ──────────────────────────────────
    pro_box = slide.shapes.add_shape(
        ROUNDED_RECTANGLE, Inches(BOX_X), Inches(PRO_Y), Inches(BOX_W), Inches(BOX_H)
    )
    pro_box.fill.background()
    pro_box.line.color.rgb = GREEN
    pro_box.line.width = Pt(1.5)

    _inject_arrow(slide, ARR_X, PRO_Y + (BOX_H - ARR_W) / 2, ARR_W, "06CA5F", rotate_180=True)

    # ── Pros: quote text ──────────────────────────────────────────
    pro_txb = slide.shapes.add_textbox(
        Inches(TXT_X), Inches(PRO_Y), Inches(TXT_W), Inches(TXT_H)
    )
    tf = pro_txb.text_frame
    tf.word_wrap = True
    tf.vertical_anchor = MSO_ANCHOR.MIDDLE
    p = tf.paragraphs[0]
    p.alignment = PP_ALIGN.LEFT
    run = p.add_run()
    run.text = (
        "“NPUs shine with low-latency, single-inference tasks — think real-time audio cleanup "
        "or live transcription during video calls. One client in the film industry reduced "
        "their audio processing time by over 70%…”"
    )
    run.font.name   = "Onest"
    run.font.size   = Pt(13)
    run.font.italic = True
    run.font.color.rgb = WHITE

    # ── Divider line ──────────────────────────────────────────────
    conn = slide.shapes.add_connector(
        MSO_CONNECTOR_TYPE.STRAIGHT,
        Inches(TXT_X), Inches(DIV_Y), Inches(TXT_X + TXT_W), Inches(DIV_Y),
    )
    conn.line.color.rgb = GRAY3
    conn.line.width = Pt(0.75)

    # ── Cons: red outlined box ────────────────────────────────────
    con_box = slide.shapes.add_shape(
        ROUNDED_RECTANGLE, Inches(BOX_X), Inches(CON_Y), Inches(BOX_W), Inches(BOX_H)
    )
    con_box.fill.background()
    con_box.line.color.rgb = REDCON
    con_box.line.width = Pt(1.5)

    _inject_arrow(slide, ARR_X, CON_Y + (BOX_H - ARR_W) / 2, ARR_W, "E4203C", rotate_180=False)

    # ── Cons: quoted source stat + original point ─────────────────
    con_txb = slide.shapes.add_textbox(
        Inches(TXT_X), Inches(CON_Y), Inches(TXT_W), Inches(TXT_H)
    )
    tf = con_txb.text_frame
    tf.word_wrap = True
    tf.vertical_anchor = MSO_ANCHOR.MIDDLE

    p = tf.paragraphs[0]
    p.alignment = PP_ALIGN.LEFT
    run = p.add_run()
    run.text   = "“memory bandwidth matters more than raw compute " \
                 "for many NPU workloads”"
    run.font.name   = "Onest"
    run.font.size   = Pt(13)
    run.font.italic = True
    run.font.color.rgb = WHITE

    p2 = tf.add_paragraph()
    p2.alignment = PP_ALIGN.LEFT
    run2 = p2.add_run()
    run2.text = "Added complexity of communication between the CPU and NPU"
    run2.font.name  = "Onest"
    run2.font.size  = Pt(13)
    run2.font.color.rgb = WHITE

    # ── Source footnote ───────────────────────────────────────────
    add_label(
        slide, 0.68, 6.78, 11.97, 0.22,
        "Source: ordinarytech.ca — On-device AI in 2026: How NPUs are Transforming AI PCs for Creators and Power Users",
        GRAY1, font_size=7.5, align=PP_ALIGN.LEFT,
        url="https://ordinarytech.ca/blogs/news/on-device-ai-in-2026-how-npus-are-transforming-ai-pcs-for-creators-and-power-users-1",
    )

    return slide


# ── Section 2 slides ─────────────────────────────────────────────────────────

def add_qnx_everywhere_slide(prs):
    """S2.1 — QNX Everywhere initiative: JUCE as a natural porting target."""
    lyt   = get_layout(prs, 1, "One Content")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "QNX Everywhere")
    set_content_ph(slide, 1,
        "QNX's initiative to run on any CPU, any board, any use case",
        [
            "Elk Audio OS already runs headless JUCE on a real-time OS — "
            "that part is solved",
            "Our contribution: full graphics support — "
            "JUCE with its complete UI running on QNX",
            "SurgeXT with its full interface on a Raspberry Pi 5 running QNX "
            "is the proof",
            "The port is the foundation — everything else in this talk builds on it",
        ],
    )
    # QNX Everywhere logo lockup — bottom-right corner (1058×204 native, ratio ~5.18:1)
    # Recolour black fills to white so the lockup is legible on the dark background.
    logo = os.path.join(SLIDES_DIR, "qnx-everywhere-logo-lockup.svg")
    if os.path.exists(logo):
        add_svg_image(slide, logo, left_in=0.68, top_in=6.60, width_in=2.70, height_in=0.52,
                      fill_subs={b'fill="black"': b'fill="white"'})
    return slide


def _patch_video_timing(slide, autoplay=True, fullscreen=True):
    """
    Modify the <p:timing> that add_movie() already wrote onto the slide.
    fullscreen=True → adds fullScrn="1" to <p:video> (PowerPoint plays full-screen).
    autoplay=True   → changes delay="indefinite" to delay="0" so playback starts
                      on slide entry rather than waiting for a click.
    """
    from pptx.oxml.ns import qn
    timing = slide._element.find(qn("p:timing"))
    if timing is None:
        return
    if fullscreen:
        for video_el in timing.iter(qn("p:video")):
            video_el.set("fullScrn", "1")
    if autoplay:
        for cond in timing.iter(qn("p:cond")):
            if cond.get("delay") == "indefinite":
                cond.set("delay", "0")


def add_video_slide(prs, video_path, title=None, poster_path=None):
    """
    Embed a video centred in the slide with an optional poster frame.
    title=None removes the title placeholder for a clean full-canvas look.
    If video_path doesn't exist, renders a branded placeholder box instead.
    Video is sized at ~70% of the slide width (maintains 16:9).
    Encoding requirements: H.264 baseline/main, yuv420p, AAC, -movflags +faststart.
    """
    lyt   = get_layout(prs, 1, "Only Title")
    slide = prs.slides.add_slide(lyt)

    if title is None:
        # Delete the title placeholder so the slide is a clean canvas.
        for ph in list(slide.placeholders):
            if ph.placeholder_format.idx == 0:
                ph._element.getparent().remove(ph._element)
                break
    else:
        set_ph(slide, 0, title)

    # ── video geometry (70% width, 16:9, centred in full slide) ──────
    VW = Inches(9.0)
    VH = Inches(5.06)   # 16:9
    VX = (Inches(13.33) - VW) / 2
    VY = (Inches(7.5)   - VH) / 2

    if os.path.exists(video_path):
        slide.shapes.add_movie(
            video_path, VX, VY, VW, VH,
            poster_frame_image=poster_path,
            mime_type="video/mp4",
        )
        _patch_video_timing(slide, autoplay=True, fullscreen=True)
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
    """S2.3 — What was quick vs what was genuinely hard in the JUCE port."""
    lyt   = get_layout(prs, 1, "Two Contents")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "Lessons from the Port")
    set_content_ph(slide, 1,
        "What worked quickly",
        [
            "ALSA — QNX supports it natively; "
            "existing Linux backend adapted in one day",
            "juce_core — POSIX base transferred almost directly",
            "Tracktion Engine — ported the same day as JUCE audio",
        ],
    )
    set_content_ph(slide, 2,
        "What took time",
        [
            "The Screen Framework — new ComponentPeer from scratch",
            "11 commits · 2,433 lines · 2+ weeks",
            "Layers: Screen FW → keyboard/mouse → fonts → "
            "OpenGL → secondary windows",
        ],
    )
    return slide


def add_screen_framework_slide(prs):
    """
    S2.4 — Architecture diagram: how JUCE connects to the QNX Screen Framework.

    Three-tier layout (top → bottom):
      JUCE APPLICATION  →  QNX PORT (our code)  →  QNX SCREEN FW + EGL

    Three columns (left → right):
      Windowing path  |  Event dispatch path  |  OpenGL / rendering path

    Connections mirror the actual code: QnxComponentPeer creates screen_window_t,
    SharedQnxScreenEventThread polls screen_get_event() on screen_context_t,
    OpenGLContext::NativeContext attaches an EGL surface to the same screen_window_t.
    """
    lyt   = get_layout(prs, 1, "Only Title")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "JUCE ↔ QNX: The Screen Architecture")

    # ── geometry ──────────────────────────────────────────────────────────
    # Three equal columns across the content area (x 0.68 → 12.65, w 11.97")
    NW     = 3.65                      # node width (all columns equal)
    COL_X  = [0.68, 4.67, 8.66]       # left edge of each column
    CX     = [x + NW / 2 for x in COL_X]   # horizontal centre of each column

    ROW_Y  = [1.90, 3.20, 4.50]       # top edge of each row
    NH     = [0.68, 0.80, 0.90]       # row node heights

    SEP_Y  = [2.95, 4.28]             # band-separator line y values

    # ── band separator lines ───────────────────────────────────────────────
    for sy in SEP_Y:
        c = slide.shapes.add_connector(
            MSO_CONNECTOR_TYPE.STRAIGHT,
            Inches(0.50), Inches(sy), Inches(12.83), Inches(sy),
        )
        c.line.color.rgb = GRAY3
        c.line.width = Pt(0.5)

    # ── band labels ────────────────────────────────────────────────────────
    add_label(slide, 0.68, 1.70, 5.0, 0.18,
              "JUCE APPLICATION", GRAY1, font_size=7.5, bold=True)
    add_label(slide, 0.68, 2.98, 8.5, 0.18,
              "QNX PORT  —  juce_Windowing_qnx.cpp  +  juce_OpenGL_qnx.h",
              RED, font_size=7.5, bold=True)
    add_label(slide, 0.68, 4.31, 4.5, 0.18,
              "QNX SCREEN FRAMEWORK", GREEN, font_size=7.5, bold=True)
    add_label(slide, COL_X[2], 4.31, NW, 0.18,
              "EGL / OpenGL ES", PURPLE, font_size=7.5, bold=True)

    # ── Row 1 — JUCE Application (GRAY3) ──────────────────────────────────
    add_node(slide, COL_X[0], ROW_Y[0], NW, NH[0],
             ["juce::Component"], GRAY3, font_size=14)
    # centre column intentionally empty in row 1
    add_node(slide, COL_X[2], ROW_Y[0], NW, NH[0],
             ["juce::OpenGLContext"], GRAY3, font_size=14)

    # ── Row 2 — Our port implementation (RED / RED2) ───────────────────────
    add_node(slide, COL_X[0], ROW_Y[1], NW, NH[1],
             ["QnxComponentPeer", "(: ComponentPeer)"], RED, font_size=11)
    add_node(slide, COL_X[1], ROW_Y[1], NW, NH[1],
             ["SharedQnxScreen", "EventThread"], RED2, font_size=11)
    add_node(slide, COL_X[2], ROW_Y[1], NW, NH[1],
             ["OpenGLContext::", "NativeContext"], RED, font_size=11)

    # ── Row 3 — OS / API layer: dark fill + coloured border ───────────────
    def _os_node(x, y, w, h, lines, border, fs=10):
        sh = slide.shapes.add_shape(
            ROUNDED_RECTANGLE, Inches(x), Inches(y), Inches(w), Inches(h)
        )
        sh.fill.solid()
        sh.fill.fore_color.rgb = DARK1
        sh.line.color.rgb      = border
        sh.line.width          = Pt(1.25)
        tf = sh.text_frame
        tf.word_wrap = True
        tf.vertical_anchor = MSO_ANCHOR.MIDDLE
        for i, ln in enumerate(lines):
            p   = tf.paragraphs[0] if i == 0 else tf.add_paragraph()
            p.alignment = PP_ALIGN.CENTER
            run = p.add_run()
            run.text           = ln
            run.font.name      = "Onest"
            run.font.size      = Pt(fs)
            run.font.color.rgb = WHITE
        return sh

    _os_node(COL_X[0], ROW_Y[2], NW, NH[2],
             ["screen_context_t", "(shared, ref-counted)"], GREEN)
    _os_node(COL_X[1], ROW_Y[2], NW, NH[2],
             ["screen_window_t", "(windowing + EGL surface)"], GREEN)
    _os_node(COL_X[2], ROW_Y[2], NW, NH[2],
             ["EGLDisplay + EGLSurface", "+ eglSwapBuffers()"], PURPLE)

    # ── Vertical connectors ────────────────────────────────────────────────
    # Left column: Component → QnxComponentPeer → screen_context_t
    add_vline(slide, CX[0], ROW_Y[0] + NH[0], ROW_Y[1], GRAY1, 1.0)
    add_vline(slide, CX[0], ROW_Y[1] + NH[1], ROW_Y[2], GRAY1, 1.0)
    # Right column: OpenGLContext → NativeContext → EGLSurface
    add_vline(slide, CX[2], ROW_Y[0] + NH[0], ROW_Y[1], GRAY1, 1.0)
    add_vline(slide, CX[2], ROW_Y[1] + NH[1], ROW_Y[2], GRAY1, 1.0)
    # Centre column: SharedEventThread → screen_context_t
    # (EventThread holds a ref to screen_context_t via SharedQnxScreenContext)
    add_vline(slide, CX[1], ROW_Y[1] + NH[1], ROW_Y[2], GRAY1, 1.0)

    # ── Horizontal: QnxComponentPeer ↔ SharedEventThread (registerWindow) ─
    mid_r2 = ROW_Y[1] + NH[1] / 2
    h_conn = slide.shapes.add_connector(
        MSO_CONNECTOR_TYPE.STRAIGHT,
        Inches(COL_X[0] + NW), Inches(mid_r2),
        Inches(COL_X[1]),       Inches(mid_r2),
    )
    h_conn.line.color.rgb = GRAY1
    h_conn.line.width     = Pt(1.0)

    # ── Relationship micro-labels ──────────────────────────────────────────
    add_label(slide, CX[0] - 0.80, ROW_Y[0] + NH[0] + 0.02, 1.60, 0.20,
              "createPeer()", GRAY1, font_size=7)
    add_label(slide, CX[2] - 0.80, ROW_Y[0] + NH[0] + 0.02, 1.60, 0.20,
              "attaches to", GRAY1, font_size=7)
    add_label(slide, COL_X[0] + NW + 0.02, mid_r2 - 0.22, 0.90, 0.20,
              "register", GRAY1, font_size=6.5)
    # NativeContext calls peer->getNativeHandle() to get screen_window_t —
    # noted as a sub-label below NativeContext rather than a diagonal line
    add_label(slide, CX[2] - 1.10, ROW_Y[1] + NH[1] + 0.03, 2.20, 0.20,
              "↓ getNativeHandle() → screen_window_t", PURPLE, font_size=7)

    return slide


def add_midi_osc_bridge_slide(prs):
    """
    S5.1 — The Bridge as the vehicle that 'got to the target'.
    Host-side NPU access + 3 identical chips = incremental dev without
    remote-dev and novel-hardware problems hitting at the same time.
    """
    lyt   = get_layout(prs, 1, "Two Contents")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "The Bridge — How We Got to the Target")
    set_content_ph(slide, 1,
        "One tool, four stages",
        [
            "Stage 1 — MIDI gap: Bridge translates Host MIDI → Target OSC. "
            "SurgeXT already speaks OSC — no changes needed",
            "Stage 2 — Voice on Host: add whisper.cpp. "
            "Same OSC path, new input source",
            "Stage 3 — Host NPU: BRIDGE_HAS_HAILO=1. "
            "Same app, Hailo backend",
            "Stage 4 — Move to Target: redeploy Bridge to RPi5. "
            "Nothing else changes",
        ],
    )
    set_content_ph(slide, 2,
        "Why 3 NPUs made this possible",
        [
            "Hailo M.2: same chip in dev laptop "
            "and in the RPi5 M.2 hat",
            "Validate NPU inference on the host — "
            "no SSH, no cross-compile, no target in the loop",
            "Never fight remote-dev issues and novel hardware "
            "at the same time",
            "One NPU per stage: Windows → Ubuntu → RPi5",
        ],
    )
    return slide


def add_tracktion_engine_slide(prs):
    """S2.6 — Tracktion Engine on QNX: the engine underneath, not a DAW."""
    lyt   = get_layout(prs, 1, "One Content")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "Bonus: Tracktion Engine on QNX")
    set_content_ph(slide, 1,
        "Timeline-based audio engine running on target useful for eval",
        [
            "“Why You Shouldn’t Write a DAW” — "
            "David Rowland, ADC23 — exactly how we’re thinking about this",
            "No MIDI on QNX? OSC over a network cable is a closer protocol fit "
            "to automotive data buses than MIDI ever was",
            "OSC is then a placeholder for real automotive data streams like VIN",
        ],
    )
    add_label(
        slide, 0.68, 6.78, 11.97, 0.22,
        "Ref: youtube.com/watch?v=GMlnh6_9aTc",
        GRAY1, font_size=7.5, align=PP_ALIGN.LEFT,
        url="https://www.youtube.com/watch?v=GMlnh6_9aTc",
    )
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


# ── Section 3 slides ─────────────────────────────────────────────────────────

def add_hailo_hardware_slide(prs):
    """S3.2 — 3x Hailo 10-H NPUs: same chip, three hardware contexts."""
    lyt   = get_layout(prs, 1, "Three Contents")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "The Hailo 10-H — One Chip, Three Contexts")
    set_content_ph(slide, 1,
        "Windows 11 Dev Laptop",
        [
            "M.2 2280 slot",
            "HailoRT SDK natively on Windows",
            "Early Hailo API experimentation",
            "Hailo backend disabled in this config",
        ],
    )
    set_content_ph(slide, 2,
        "Ubuntu Dev Laptop",
        [
            "M.2 2280 slot",
            "Full HailoRT + arm64 cross-sysroot",
            "Primary NPU pipeline dev host",
            "Cross-compiles the Hailo binary for RPi5",
        ],
    )
    set_content_ph(slide, 13,
        "Raspberry Pi 5",
        [
            "M.2 2280 hat via PCIe",
            "Hailo 10-H runs Whisper on-device",
            "The deployment target",
            "Same M.2 form factor across all three",
        ],
    )
    return slide


def add_npu_speed_result_slide(prs):
    """S3.3 — The headline result: CPU vs Hailo 10-H on Whisper inference."""
    lyt   = get_layout(prs, 1, "Two Contents")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "10× Faster on the NPU")
    set_content_ph(slide, 1,
        "Whisper on CPU — baseline",
        [
            "RPi5 ARM cores fully loaded during inference",
            "Transcription latency: [measured — TODO]",
            "SurgeXT audio competes for the same cores",
            "Not practical for live performance",
        ],
    )
    set_content_ph(slide, 2,
        "Whisper on Hailo 10-H",
        [
            "Same model, same audio input",
            "~10× faster than CPU baseline",
            "CPU load during inference: near zero",
            "SurgeXT runs undisturbed — real-time achieved",
        ],
    )
    return slide


# ── Section 4 slides ─────────────────────────────────────────────────────────

def add_whispercpp_testbench_slide(prs):
    """S4.1 — whisper.cpp testbench on the host CPU: validating the approach first."""
    lyt   = get_layout(prs, 1, "One Content")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "The whisper.cpp Testbench")
    set_content_ph(slide, 1,
        "Validate the full Voice → MIDI chain on CPU before touching the NPU",
        [
            "whisper.cpp: portable C++ Whisper — no Python, no GPU",
            "Develops and debugs entirely on Windows or Ubuntu laptop",
            "CPU baseline timing recorded here — the 'before' in the 10× story",
            "The NPU is never the variable until the logic is proven",
        ],
    )
    return slide


def add_voice_midi_chain_slide(prs):
    """S4.2 — How voice commands map to MIDI Program + Bank Change messages."""
    lyt   = get_layout(prs, 1, "Two Contents")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "Voice → MIDI: The Chain")
    set_content_ph(slide, 1,
        "The signal path",
        [
            "Voice → whisper.cpp → transcribed text",
            "Text matched against SurgeXT patch names",
            "Match → MIDI Program Change + Bank Change",
            "OSC bridge → SurgeXT loads the patch",
        ],
    )
    set_content_ph(slide, 2,
        "Why MIDI Program + Bank Change?",
        [
            "SurgeXT has no headless patch-selection API",
            "Prog+Bank is the standard synth patch address space",
            "Covers the entire SurgeXT factory catalogue",
            "No SurgeXT source modifications required",
        ],
    )
    return slide


# ── Punchline (between Section 5 and Section 6) ───────────────────────────────

def add_punchline_slide(prs):
    """
    The payoff moment — spoken BEFORE Video clip 4 plays.
    Uses the Statement layout for full-slide impact.
    """
    lyt   = get_layout(prs, 1, "Statement")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0,  "The Punchline")
    set_ph(slide, 16, (
        "All of this engineering…\n"
        "was just so I could browse SurgeXT patches\n"
        "while playing with both hands."
    ))
    return slide


# ── Section 6 slides ─────────────────────────────────────────────────────────

def add_lessons_slide(prs):
    """S6.1 — Three honest things that were harder than expected."""
    lyt   = get_layout(prs, 1, "Three Contents")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "Harder Than Expected")
    set_content_ph(slide, 1,
        "QNX Has No MIDI",
        [
            "MIDI has no role in automotive — QNX simply doesn't have it",
            "SurgeMidiToOscBridge: a prerequisite, not a side quest",
            "OSC over UDP: cleaner than raw MIDI in the end",
            "The blocker is sometimes the thing you needed to build",
        ],
    )
    set_content_ph(slide, 2,
        "Hailo vs the GPU World",
        [
            "GPU inference: years of tooling, run-anything-on-CUDA",
            "Hailo: better than Qualcomm, steeper than GPU ecosystem",
            "Model → HEF conversion: a step GPU workflows don't need",
            "Worth it for constrained-hardware latency — budget the ramp",
        ],
    )
    set_content_ph(slide, 13,
        "Cross-Compilation Rabbit Holes",
        [
            "Ubuntu apt multiarch: [arch=amd64] qualifiers + "
            "ports.ubuntu.com arm64 lines",
            "DEB822 .sources format is cleaner — if you know it exists",
            "QNX SDP: easier — cross-compile is the expected workflow",
            "Invest a day in the dev environment, "
            "not a week fighting the wrong variable",
        ],
    )
    return slide


def add_next_steps_slide(prs):
    """S6.2 — Three directions this work opens up."""
    lyt   = get_layout(prs, 1, "Three Contents")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "What This Opens Up")
    set_content_ph(slide, 1,
        "NeuralPlayer: What Happened Next",
        [
            "ADCx'23: Stem Separation + Audio-to-MIDI on the host",
            "It didn't stop there",
            "A future talk — the clue is the boundary line is gone",
            "(TX/RX Part 3?)",
        ],
    )
    set_content_ph(slide, 2,
        "More NPU Workloads",
        [
            "Stem separation: real-time on the NPU, no cloud",
            "AI audio enhancement: de-noise, de-reverb on-device",
            "Multiple model pipelines on one Hailo chip",
            "Edge audio AI — no cloud, no GPU, no Wi-Fi",
        ],
    )
    set_content_ph(slide, 13,
        "JUCE on QNX, Continued",
        [
            "Contributing the port upstream to JUCE",
            "QNX Everywhere: the port widens its hardware reach",
            "Tracktion Engine: full DAW-class pipelines on QNX",
            "The embedded audio ecosystem is opening up",
        ],
    )
    return slide


# ── Dedication slide ──────────────────────────────────────────────────────────

def add_dedication_slide(prs):
    """
    Post-punchline dedication to Jason Dasent — music producer and accessibility
    consultant whose vision, sparked by ADC21, is what this project is reaching for.
    """
    lyt   = get_layout(prs, 1, "Statement")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "Dedicated to")
    set_ph(slide, 12, (
        "Jason Dasent — music producer and accessibility consultant —\n"
        "saw ADC21 and asked: can a Raspberry Pi and voice commands\n"
        "help blind people work in a professional studio?\n"
        "\n"
        "It took five years. This project is the beginning of that answer.\n"
        "\n"
        "The FOSS community is invited to help make his dream a reality."
    ))
    add_label(
        slide, 0.68, 6.78, 4.0, 0.22,
        "jasondasent.com",
        GRAY1, font_size=7.5, align=PP_ALIGN.LEFT,
        url="https://www.jasondasent.com/",
    )
    return slide


# ── Contact slide ─────────────────────────────────────────────────────────────

def add_contact_slide(prs):
    """Final slide — Contact card layout (potx slide 14)."""
    lyt   = get_layout(prs, 1, "Contact")
    slide = prs.slides.add_slide(lyt)
    set_ph(slide, 0, "Thank you")

    # ── Contact details (ph=10): name / title / details ───────────
    for ph in slide.placeholders:
        try:
            if ph.placeholder_format.idx != 10:
                continue
        except Exception:
            continue

        tf = ph.text_frame
        tf.word_wrap = True

        p = tf.paragraphs[0]
        p.clear()
        p.level = 0
        p.add_run().text = "Kieran Coulter"

        p2 = tf.add_paragraph()
        p2.level = 1
        p2.add_run().text = "Principal Systems Software Engineer - Audio"

        p3 = tf.add_paragraph()
        p3.level = 2
        p3.add_run().text = "Phone\t+1 604 319 1613"

        p4 = tf.add_paragraph()
        p4.level = 2
        p4.add_run().text = "E-Mail\tkicoulter@qnx.com"

        p5 = tf.add_paragraph()
        p5.level = 2
        p5.add_run().text = "201-8331 Eastlake Drive"

        p6 = tf.add_paragraph()
        p6.level = 2
        p6.add_run().text = "Burnaby  BC  V5A 4W2"

        p7 = tf.add_paragraph()
        p7.level = 2
        p7.add_run().text = "Canada"

        break

    # ── Photo: fit-within-bounds, top-aligned, no cropping ────────
    # insert_picture() zooms to fill width and crops the height, which cuts
    # the top of a portrait photo off. Instead, scale to fit within the
    # placeholder area and top-align so the head is always fully visible.
    photo_path = os.path.join(SLIDES_DIR, "KieranCoulter.jpg")
    if os.path.exists(photo_path):
        ph13 = None
        for ph in slide.placeholders:
            try:
                if ph.placeholder_format.idx == 13:
                    ph13 = ph
                    break
            except Exception:
                continue

        if ph13 is not None:
            ph_left, ph_top = ph13.left, ph13.top
            ph_w,    ph_h   = ph13.width, ph13.height
            img_w_px, img_h_px = _image_dims(photo_path)

            if img_w_px and img_h_px:
                scale = min(ph_w / img_w_px, ph_h / img_h_px)
                pic_w = int(img_w_px * scale)
                pic_h = int(img_h_px * scale)
                # Centre horizontally, align to top of placeholder area
                pic_left = ph_left + (ph_w - pic_w) // 2
                pic_top  = ph_top
                slide.shapes.add_picture(photo_path, pic_left, pic_top, pic_w, pic_h)
            else:
                ph13.insert_picture(photo_path)   # fallback
    else:
        print(f"Note: {photo_path} not found — photo placeholder left empty")

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
            ("Coming in Future Talk",
             "NeuralPlayer didn't stop here — a future talk will reveal where this project went next"),
        ],
    )

    # ── Section 2: JUCE on QNX ───────────────────────────────────
    add_divider(prs, 2, "JUCE on QNX")
    add_qnx_everywhere_slide(prs)
    add_video_slide(
        prs,
        video_path=os.path.join(SLIDES_DIR, "placeholder.mp4"),
    )
    add_juce_porting_lessons_slide(prs)
    add_screen_framework_slide(prs)
    add_tracktion_engine_slide(prs)

    # ── Section 3: Audio on the NPU ──────────────────────────────
    add_divider(prs, 3, "Audio on the NPU")
    add_npu_pros_cons_slide(prs)
    add_hailo_hardware_slide(prs)
    add_npu_speed_result_slide(prs)

    # ── Section 4: Developing Host-First ─────────────────────────
    add_divider(prs, 4, "Developing Host-First")
    add_whispercpp_testbench_slide(prs)
    add_video_slide(
        prs,
        video_path=os.path.join(SLIDES_DIR, "clip2_testbench_host.mp4"),
        title="Testbench Running on Host CPU",
    )
    add_voice_midi_chain_slide(prs)

    # ── Section 5: Getting to the Target ─────────────────────────
    add_divider(prs, 5, "Getting to the Target")
    add_midi_osc_bridge_slide(prs)
    add_host_target_support_matrix_slide(prs)
    add_signal_chain_slide(prs)
    add_video_slide(
        prs,
        video_path=os.path.join(SLIDES_DIR, "clip3_npu_speedup.mp4"),
        title="CPU → NPU: The Speed Difference",
    )
    add_signal_chain_full_target_slide(prs)

    # ── Punchline (no section card — flows from Section 5) ────────
    add_punchline_slide(prs)
    add_video_slide(
        prs,
        video_path=os.path.join(SLIDES_DIR, "clip4_full_demo.mp4"),
        title="Live: Voice-Commanded Patch Selection",
    )

    # ── Dedication + Contact ──────────────────────────────────────
    add_dedication_slide(prs)
    add_contact_slide(prs)

    prs.save(OUTPUT)
    print(f"Saved {len(prs.slides)} slides: {OUTPUT}")


if __name__ == "__main__":
    main()
