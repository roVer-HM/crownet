from rover_utils.gui.gui_fuzzy_list_select import TkFuzzySelectList, FuzzyItemWrapper


def gui_select_string(data):
    return TkFuzzySelectList.get_selected(FuzzyItemWrapper.wrap_string_list(data))
