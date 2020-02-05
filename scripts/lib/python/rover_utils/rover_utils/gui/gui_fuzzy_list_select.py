
from fuzzywuzzy import process
import tkinter as tk
from rover_utils.gui import gui_common as gui

class TkFuzzySelectList(tk.Frame):
    """
    simple string list with single select item. If user starts typing (independent of focus)
    a fuzzy matching occurs to select the best match. Hit Enter (Return) to select item and
    close GUI.
    Hit Esc to return with emtpy String.
    """

    @classmethod
    def get_selected(cls, data, score_cutoff=25):
        _root = tk.Tk()
        _root.wm_attributes("-type", "dialog")
        _app = cls(data=data, score_cutoff=score_cutoff, master=_root)
        _app.focus_force()
        _app.mainloop()
        return _app.selected

    def _cfg(self, widget, **cfg):
        _options = dict()
        _options.update(gui.default_cfg)
        _options.update(cfg)
        widget.config(**_options)

    def _pack(self, widget, **pck):
        _options = dict()
        _options.update(gui.default_pack_cfg)
        _options.update(pck)
        widget.pack(**_options)

    def __init__(self, data, master=None, score_cutoff=25):
        super().__init__(master)
        self.master = master
        self._score_cutoff = score_cutoff
        self._raw_data = data
        self._list_data = tk.StringVar(value=self._raw_data)
        self.selected = ""

        self._width = 0
        for item in self._raw_data:
            if len(item) > self._width:
                self._width = len(item)
        self._width += 2

        self.config(
            highlightcolor=gui.highlightcolor,
            highlightbackground=gui.highlightbackground,
            background=gui.background,
            borderwidth=0,
            highlightthickness=0,
        )
        self._pack(self, fill=tk.BOTH)
        self.create_widgets()

    def create_widgets(self):
        self.q_str = tk.StringVar()
        self.q_val = self.register(self._fuzzy_match)
        self.search = tk.Entry()
        self._cfg(
            self.search,
            textvariable=self.q_str,
            validate="key",
            validatecommand=(self.q_val, "%P"),
            font=("Courier", 16),
        )
        self._pack(self.search)

        self.search.bind("<Return>", self._return)
        self.search.bind("<Up>", self._up, add="")
        self.search.bind("<Down>", self._down, add="")
        self.search.bind("<Escape>", self._esc)

        self.search.focus_set()

        self.list = tk.Listbox()
        self._cfg(
            self.list, listvariable=self._list_data,
            selectmode=tk.BROWSE,
            width=self._width,
        )
        self.list["listvariable"] = self._list_data
        self._pack(self.list, fill=tk.BOTH, expand=True)

        self.list.bind("<Return>", self._return)
        self.list.bind("<Escape>", self._esc)
        self.list.bind("<KeyPress>", self._key)

        self.list.select_clear(0, tk.END)
        self.list.activate(0)
        self.list.select_set(0)

    def _key(self, event):
        self.search.focus_set()

    def _esc(self, event):
        self.selected = ""
        self.master.destroy()

    def _down(self, event):
        if event.widget == self.search:
            self.list.focus_set()
            self.list.event_generate("<Down>", when="tail")

    def _up(self, event):
        if event.widget == self.search:
            self.list.focus_set()
            self.list.event_generate("<Up>", when="tail")

    def _return(self, event):
        self.selected = self.list.get(tk.ACTIVE)
        self.master.destroy()

    def _fuzzy_match(self, new_val):
        if len(new_val) > 0:
            q_result = [
                item[0]
                for item in process.extractBests(new_val, self._raw_data, score_cutoff=25)
            ]
            self._list_data.set(q_result)
        else:
            self._list_data.set(self._raw_data)

        if self.list.size() > 0:
            self.list.select_clear(0, tk.END)
            self.list.activate(0)
            self.list.select_set(0)
        return True


if __name__ == "__main__":
    app = TkFuzzySelectList.get_selected(['aa', 'b', 'baa', 'bcc', 'c', 'd'])
    app.mainloop()

