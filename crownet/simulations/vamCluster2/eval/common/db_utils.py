import os
import glob

def get_vec_files(results, study):
    files = []
    for result in os.listdir(f"{results}/{study}"):
        files.extend(glob.glob(f"{results}/{study}/{result}/*.vec"))
    return files

def get_parameter_value(cursor, parameter):
    cursor.execute(f"SELECT configValue FROM runConfig WHERE configKey = '{parameter}'")
    return cursor.fetchone()[0]

def count_all_vams(cur, vec_name, mod_name, start, end):
    data = cur.execute(f"""
        select count(vectorData.value) from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = '{vec_name}'
        and moduleName LIKE '{mod_name}'
        and vectorData.simtimeRaw > {start}
        and vectorData.simtimeRaw < {end}
        """).fetchall()
    return data[0][0] if len(data) else None

def get_resource_blocks(cur, mod_name, start, end):
    data = cur.execute(f"""
        select vectorData.value, vectorData.simtimeRaw from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'avgServedBlocksUl:vector'
        and moduleName LIKE '{mod_name}'
        and vectorData.simtimeRaw > {start}
        and vectorData.simtimeRaw < {end}
        """).fetchall()
    return [(d[0], d[1]) for d in data]

def get_position_error(cur, mod_name, start, end):
    data = cur.execute(f"""
        select vectorData.value, vectorData.simtimeRaw from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'pos_error_c'
        and moduleName LIKE '{mod_name}'
        and vectorData.simtimeRaw > {start}
        and vectorData.simtimeRaw < {end}
        """).fetchall()
    return [(d[0], d[1]) for d in data]

def get_position_error_all_vams(cur, mod_name, start, end):
    data = cur.execute(f"""
        select vectorData.value, vectorData.simtimeRaw from vectorData inner join vector
        on vectorData.vectorId = vector.vectorId
        and vectorName = 'pos_error'
        and moduleName LIKE '{mod_name}'
        and vectorData.simtimeRaw > {start}
        and vectorData.simtimeRaw < {end}
        """).fetchall()
    return [(d[0], d[1]) for d in data]

def get_va_service_modules(cur, vec_name, mod_name):
    modules = cur.execute(f"""
        select moduleName, startSimtimeRaw, endSimtimeRaw, vectorCount from vector
        where moduleName LIKE '{mod_name}'
        and vectorName = '{vec_name}'
        """).fetchall()
    return [(m[0], m[1], m[2], m[3]) for m in modules if m[0] and m[1] and m[2] and m[3]]