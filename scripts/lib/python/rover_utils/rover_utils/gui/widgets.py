import tkinter as tk

from fuzzywuzzy import process

from rover_utils.gui.gui_common import Colors, config, default_cfg, pack


class FuzzyItemWrapper:
    @classmethod
    def wrap_string(cls, obj):
        return cls(obj, lambda x: x, lambda x: x, lambda x: x)

    @classmethod
    def wrap_string_list(cls, objs):
        return [FuzzyItemWrapper.wrap_string(o) for o in objs]

    @classmethod
    def wrap_number(cls, obj):
        return cls(obj, lambda x: str(x), lambda x: str(x), lambda x: str(x))

    @classmethod
    def wrap_numbers_list(cls, objs):
        return [FuzzyItemWrapper.wrap_number(o) for o in objs]

    def __init__(self, obj, id_func, display_func, search_func):
        self.element = obj
        self._id_func = id_func
        self._display_str = display_func
        self._search_str = search_func

    def __len__(self):
        return len(self.display_str())

    def display_str(self) -> str:
        return self._display_str(self.element)

    def search_str(self) -> str:
        return self._search_str(self.element)

    def id(self):
        return self._id_func(self.element)

    def get_wrapped_obj(self):
        return self.element


class FuzzyFindList(tk.Listbox):
    def __init__(self, raw_data, master=None, **kwargs):
        super().__init__(master, **kwargs)

        if type(raw_data) is not list:
            raise ValueError("expected a list")

        if len(raw_data) == 0:
            raise ValueError("expected at least one item")

        if not all(type(i) is FuzzyItemWrapper for i in raw_data):
            raise ValueError("expected list of FuzzyItemWrapper items")

        self._raw_data_dict = {item.id(): item for item in raw_data}
        self._search_dict = {k: v.search_str() for k, v in self._raw_data_dict.items()}
        self._display_data = tk.StringVar(
            value=[item.display_str() for k, item in self._raw_data_dict.items()]
        )
        self._q_result = [
            (v.display_str(), 100, k) for k, v in self._raw_data_dict.items()
        ]  # list of tuples with (display_str, score, dict_key)
        self.selected = ""

        self._width = 0
        for _, item in self._raw_data_dict.items():
            if len(item.display_str()) > self._width:
                self._width = len(item.display_str())
        self._width += 2

        # init search field
        self.query_field = tk.Entry(master=master)
        config(
            self.query_field,
            validate="key",
            validatecommand=(self.query_field.register(self._match), "%P"),
            font=("Courier", 16),
        )
        pack(self.query_field)
        self.query_field.bind("<<FuzzyFindEntrySearchUpdate>>", self._search_update)
        self.query_field.bind("<Up>", self._up, add="")
        self.query_field.bind("<Down>", self._down, add="")

        self.bind("<KeyPress>", self._key)

        config(
            self,
            listvariable=self._display_data,
            selectmode=tk.BROWSE,
            width=self._width,
        )
        self["listvariable"] = self._display_data  # needed?
        pack(self, fill=tk.BOTH, expand=True)

        self.select_clear(0, tk.END)
        self.activate(0)
        self.select_set(0)

    def reload_raw_data(self, raw_data):
        self._raw_data_dict = {item.id(): item for item in raw_data}
        self._search_dict = {k: v.search_str() for k, v in self._raw_data_dict.items()}
        self._display_data.set(
            value=[item.display_str() for k, item in self._raw_data_dict.items()]
        )
        self._q_result = [
            (v.display_str(), 100, k) for k, v in self._raw_data_dict.items()
        ]
        self.selected = ""

    def get_selected_raw(self):
        res_tuple = self._q_result[self.curselection()[0]]
        return self._raw_data_dict[res_tuple[2]].get_wrapped_obj()

    def get_selected(self) -> FuzzyItemWrapper:
        res_tuple = self._q_result[self.curselection()[0]]
        return self._raw_data_dict[res_tuple[2]]

    def _key(self, event):
        self.query_field.focus_set()

    def _down(self, event):
        if event.widget == self.query_field:
            self.focus_set()
            self.event_generate("<Down>", when="tail")

    def _up(self, event):
        if event.widget == self.query_field:
            self.focus_set()
            self.event_generate("<Up>", when="tail")

    def _search_update(self, *args):
        if self.size() > 0:
            self.select_clear(0, tk.END)
            self.activate(0)
            self.select_set(0)
        return True

    def _match(self, new_val):
        if len(new_val) > 0:
            self._q_result = process.extractBests(
                new_val, self._search_dict, score_cutoff=25
            )

            self._display_data.set(
                [
                    self._raw_data_dict[result[2]].display_str()
                    for result in self._q_result
                ]
            )
        else:
            self._display_data.set(
                [item.display_str() for k, item in self._raw_data_dict.items()]
            )

        self.event_generate("<<FuzzyFindEntrySearchUpdate>>", when="tail")
        return True


# see https://stackoverflow.com/a/47928390/12079469
class EntryWithPlaceholder(tk.Entry):
    def __init__(
        self, index, name, master=None, placeholder="PLACEHOLDER", mandatroy=False
    ):
        super().__init__(master, justify="center")

        self.index = index
        self.mandatory = mandatroy
        self.item_title = name
        if self.mandatory:
            self.placeholder = f"<{placeholder}>*"
        else:
            self.placeholder = f"<{placeholder}>"
        self.placeholder_color = default_cfg["fg"]
        self.default_fg_color = default_cfg["fg"]
        self["bg"] = default_cfg["bg"]
        self["font"] = default_cfg["font"]

        self.bind("<FocusIn>", self.foc_in)
        self.bind("<FocusOut>", self.foc_out)

        self.put_placeholder()

    def valid(self):
        if self.mandatory:
            return self.get() != "" and self.get() != self.placeholder
        else:
            return True

    def has_data(self):
        return self.get() != "" and self.get() != self.placeholder

    def get_clean(self):
        if self.get() == self.placeholder:
            return ""
        else:
            return self.get()

    def put_placeholder(self):
        self.insert(0, self.placeholder)
        self["fg"] = self.placeholder_color

    def foc_in(self, *args):
        if self.get() == self.placeholder:
            self.delete("0", "end")
            self["fg"] = self.default_fg_color

    def foc_out(self, *args):
        if not self.get():
            self.put_placeholder()


class SimpleForm(tk.Frame):
    """
    Accepts list of 1- 2- or 3-tuple with (name, required(default=false), placeholder(default=name))
    """

    def __init__(
        self,
        entries,
        with_btn=True,
        master=None,
        entry_cgf=None,
        entry_pack=None,
        **kwargs,
    ):
        super().__init__(master, **kwargs)

        self._inputs = list()
        for idx, item in enumerate(entries):
            if len(item) == 1:
                entr = EntryWithPlaceholder(
                    master=master,
                    index=idx,
                    name=item[0],
                    placeholder=item[0],
                    mandatroy=False,
                )
            elif len(item) == 2:
                entr = EntryWithPlaceholder(
                    master=master,
                    index=idx,
                    name=item[0],
                    placeholder=item[0],
                    mandatroy=item[1],
                )
            else:
                entr = EntryWithPlaceholder(
                    master=master,
                    index=idx,
                    name=item[0],
                    placeholder=item[2],
                    mandatroy=item[1],
                )

            entr.bind("<Up>", self.e_input_up, add="")
            entr.bind("<Down>", self.e_input_down, add="")
            self._inputs.append(entr)
            cfg = {"font": ("Courier", 10, "italic")}
            if entry_cgf is not None:
                cfg.update(entry_cgf)
            config(entr, **cfg)

            pck = {"fill": tk.X}
            if entry_pack is not None:
                pck.update(entry_pack)
            pack(entr, **pck)

        if with_btn:
            self.input_btn = tk.Button(text=">>>Create Glossary Entry<<<")
            config(
                self.input_btn,
                justify="center",
                background=Colors.btn_background,
                activebackground=Colors.btn_hover,
            )
            pack(self.input_btn, fill=tk.X)

    def e_input_up(self, event):
        if type(event.widget) == EntryWithPlaceholder:
            idx = (event.widget.index - 1 + len(self._inputs)) % len(self._inputs)
            self._inputs[idx].focus_set()

    def e_input_down(self, event):
        if type(event.widget) == EntryWithPlaceholder:
            idx = (event.widget.index + 1) % len(self._inputs)
            self._inputs[idx].focus_set()

    def bind_for_each(self, event, callback):
        for i in self._inputs:
            i.bind(event, callback)

    def is_valid(self):
        for item in self._inputs:
            if not item.valid():
                return False
        return True

    def get(self):
        data = {}
        if self.is_valid():
            for item in self._inputs:
                data[item.item_title] = item.get_clean()
        return data


if __name__ == "__main__":
    pass
