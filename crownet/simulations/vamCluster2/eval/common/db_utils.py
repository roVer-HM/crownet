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

def get_vru_pos(cur, mod_name, axis="x"):
    data = cur.execute(f"""
        select vdx.value, vdx.simtimeRaw
        from vector v inner join vectorData vdx
            on v.vectorId = vdx.vectorId
        where v.vectorName = 'self_{axis}'
            and v.moduleName LIKE '{mod_name}'
        """).fetchall()
    return [(d[0], d[1]) for d in data]

def aggregate_vam_recv(cur, mod_name):
    data = cur.execute(f"""
        select v.vectorName, vd.value, vd.simtimeRaw
        from vector v
        inner join vectorData vd
            on v.vectorId = vd.vectorId
                and ( v.vectorName = 'vam_x'
                    or v.vectorName = 'vam_y'
                    or v.vectorName = 'vam_id'
                    or v.vectorName = 'vam_cluster'
                    or v.vectorName = 'pos_error_c'
                    or v.vectorName = 'vam_bbox_size' )
        where v.moduleName LIKE '{mod_name}'
    """)

    aggregated_data = {}

    for d in data:
        # Simtime
        if (d[2] not in aggregated_data):
            aggregated_data[d[2]] = {}
        # Vector Name
        if (d[0] not in aggregated_data[d[2]]):
            aggregated_data[d[2]][d[0]] = d[1]

    return aggregated_data