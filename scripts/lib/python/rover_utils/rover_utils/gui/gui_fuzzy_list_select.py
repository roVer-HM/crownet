import tkinter as tk

from rover_utils.gui.gui_common import Colors, pack
from rover_utils.gui.widgets import FuzzyFindList, FuzzyItemWrapper


class TkFuzzySelectList(tk.Frame):
    """
    simple string list with single select item. If user starts typing (independent of focus)
    a fuzzy matching occurs to select the best match. Hit Enter (Return) to select item and
    close GUI.
    Hit Esc to return with emtpy String.
    """

    @classmethod
    def get_selected(cls, data, score_cutoff=25):
        _root = tk.Tk(className="Select Path | Enter: Select  Esc: Close")
        _root.wm_attributes("-type", "dialog")
        _app = cls(data=data, score_cutoff=score_cutoff, master=_root)
        _app.focus_force()
        _app.mainloop()
        return _app.selected

    def __init__(self, data, master=None, score_cutoff=25):
        super().__init__(master)
        self.master = master
        self.selected = ""
        self.config(
            highlightcolor=Colors.highlightcolor,
            highlightbackground=Colors.highlightbackground,
            background=Colors.background,
            borderwidth=0,
            highlightthickness=0,
        )
        pack(self, fill=tk.BOTH)
        self.master.bind_all("<Escape>", self._esc)
        self.master.bind("<Return>", self._return)

        self.fuzzy_list = FuzzyFindList(raw_data=data, master=master)
        self.fuzzy_list.query_field.focus_set()
        pack(self.fuzzy_list, fill=tk.BOTH, expand=True)

    def _esc(self, event):
        self.selected = ""
        self.master.destroy()

    def _return(self, event):
        self.selected = self.fuzzy_list.get_selected_raw()
        self.master.destroy()


if __name__ == "__main__":
    app = TkFuzzySelectList.get_selected(
        FuzzyItemWrapper.wrap_string_list(["aa", "b", "baa", "bcc", "c", "d"])
    )
    # app = TkFuzzySelectList.get_selected(FuzzyItemWrapper.wrap_numbers_list([44, 23.2, 35, 5345, -532]))
    print(f"selected: {app}")
