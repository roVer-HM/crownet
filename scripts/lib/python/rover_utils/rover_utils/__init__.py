from rover_utils.gui.gui_fuzzy_list_select import TkFuzzySelectList


def gui_select(data):
    return TkFuzzySelectList.get_selected(data)
