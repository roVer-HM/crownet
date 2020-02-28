class Colors:
    highlightcolor = "#3C3F41"
    highlightbackground = "#3C3F41"
    background = "#3C3F41"
    foreground = "#AFB1B3"
    btn_hover = "#87939A"
    btn_background = "#2A2D2E"


default_pack_cfg = {"side": "top", "fill": "x"}
default_cfg = {
    "bg": Colors.background,
    "fg": Colors.foreground,
    "font": ("Courier", 12),
}


def config(widget, **cfg):
    _options = dict()
    _options.update(default_cfg)
    _options.update(cfg)
    widget.config(**_options)


def pack(widget, **pck):
    _options = dict()
    _options.update(default_pack_cfg)
    _options.update(pck)
    widget.pack(**_options)
