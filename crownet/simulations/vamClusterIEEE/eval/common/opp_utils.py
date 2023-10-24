ST_CONV_FACTOR = 10**12

def st_to_s(st):
    return st /  ST_CONV_FACTOR

def s_to_st(s):
    return s *  ST_CONV_FACTOR