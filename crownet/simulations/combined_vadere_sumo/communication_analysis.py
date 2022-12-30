import sqlite3
import matplotlib.pyplot as plt
import numpy as np
import os
import glob
import os.path as path

def extract_sim_data(sca_file, vec_file):
    data = {}
    with sqlite3.connect(sca_file) as s:
        cur = s.cursor()

        configname = cur.execute("""
            SELECT attrValue
            FROM runAttr
            WHERE attrName = 'configname'
        """).fetchone()[0]

        data["p_mobility"] = "vadere" if ("Combined" in configname) else "sumo"

        data["generation_interval"] = float(cur.execute("""
            SELECT configValue
            FROM runConfig
            WHERE configKey = '*.*Node[*].app[0].scheduler.generationInterval'
            ORDER BY configOrder
            LIMIT 1
        """).fetchone()[0].split("s")[0])

        data["received_packets_p"] = cur.execute("""
            SELECT moduleName, scalarValue
            FROM scalar
            WHERE scalarName = 'receivedPacketFromLowerLayer:count'
                AND moduleName LIKE '%pNode%.cellularNic.mac'
        """).fetchall()

        data["received_packets_v"] = cur.execute("""
            SELECT moduleName, scalarValue
            FROM scalar
            WHERE scalarName = 'receivedPacketFromLowerLayer:count'
                AND moduleName LIKE '%vNode%.cellularNic.mac'
        """).fetchall()
        
        data["received_packets_p_appl"] = cur.execute("""
            SELECT moduleName, scalarValue
            FROM scalar
            WHERE scalarName = 'packetReceived:count'
                AND moduleName LIKE '%pNode%.udp'
        """).fetchall()

        data["received_packets_v_appl"] = cur.execute("""
            SELECT moduleName, scalarValue
            FROM scalar
            WHERE scalarName = 'packetReceived:count'
                AND moduleName LIKE '%vNode%.udp'
        """).fetchall()

    with sqlite3.connect(vec_file) as s:
        cur = s.cursor()

        # data["sinr"] = cur.execute("""
        #     SELECT vd.value, v.moduleName
        #     FROM vector v
        #     INNER JOIN vectorData vd
        #         ON vd.vectorId = v.vectorId
        #         AND v.vectorName = 'rcvdSinrUl:vector'
        #         AND v.moduleName LIKE '%.cellularNic.channelModel[0]'
        # """).fetchall()

    return data


def analyze_received_packets(input_data):
    recv_packets = {
        "sumo": {},
        "vadere": {},
    }
    recv_packets_app = {
        "sumo": {},
        "vadere": {},
    }

    for data in input_data:
        simulator = data["p_mobility"]
        interval = f"{int(data['generation_interval'] * 100)}"

        if interval not in recv_packets[simulator]:
            recv_packets[simulator][interval] = {"p": [], "v": []}
            recv_packets_app[simulator][interval] = {"p": [], "v": []}

        for p in data["received_packets_p"]:
            recv_packets[simulator][interval]["p"].append(p[1])

        for v in data["received_packets_v"]:
            recv_packets[simulator][interval]["v"].append(v[1])
            
        for p in data["received_packets_p_appl"]:
            recv_packets_app[simulator][interval]["p"].append(p[1])

        for v in data["received_packets_v_appl"]:
            recv_packets_app[simulator][interval]["v"].append(v[1])
            
            
            
    def plt_dict_errorbar(dict, node_key, fmt):
        d_x = list([int(k) for k in dict.keys()])
        d_x.sort()
        d_x = [str(x) for x in d_x]
        d_mean = [np.mean(dict[x][node_key]) for x in d_x]
        d_err = [np.std(dict[x][node_key]) for x in d_x]
        
        
        plt.errorbar([int(x) for x in d_x], d_mean, yerr=d_err, fmt=fmt, capsize=5, markersize=20)
 
    def newplt(layer):
        fig = plt.figure()
        fig.set_size_inches(9, 9)
        fig.subplots_adjust(left=0.16)
        plt.rc('font', size=28)
        
            
        plt.xticks([20, 40, 60, 80, 100], ["20", "40", "60", "80", "100"])
        plt.yticks([50000, 100000, 150000, 200000], ["50", "100", "150", "200"])
    
        plt.ylabel(f"# Received Packets ($10^3$, {layer} layer)")
        plt.xlabel("Inter-TX Time [ms]")
        
        
    def saveplt(name):
        plt.savefig(f"/home/rupp/{name}.png")
        plt.savefig(f"/home/rupp/{name}.svg")
        plt.savefig(f"/home/rupp/{name}.pdf")      

    newplt("link")
    plt_dict_errorbar(recv_packets["sumo"], "p","o")
    plt_dict_errorbar(recv_packets["vadere"], "p","*")
    plt.legend(["sumo: striping", "vadere: OSM"])
    saveplt("inter_tx_pNode_mac")

    
    newplt("link")
    plt_dict_errorbar(recv_packets["sumo"], "v","o")
    plt_dict_errorbar(recv_packets["vadere"], "v","*")
    plt.legend(["sumo: striping", "vadere: OSM"])
    saveplt("inter_tx_vNode_mac")

    
    newplt("app.")
    plt_dict_errorbar(recv_packets_app["sumo"], "p","o")
    plt_dict_errorbar(recv_packets_app["vadere"], "p","*")
    plt.legend(["sumo: striping", "vadere: OSM"])
    saveplt("inter_tx_pNode_app")

    
    newplt("app.")
    plt_dict_errorbar(recv_packets_app["sumo"], "v","o")
    plt_dict_errorbar(recv_packets_app["vadere"], "v","*")
    plt.legend(["sumo: striping", "vadere: OSM"])
    saveplt("inter_tx_vNode_app")


if __name__ == '__main__':
    
    basedir = "/home/rupp/crownet_paper/crownet/simulations/combined_vadere_sumo/results"
    
    datas = []
    
    for sub in os.listdir(basedir):
        result_dir = path.join(basedir, sub)
        if path.isdir(result_dir) and "Study" in sub:
            sca_files = glob.glob(f"{result_dir}/*.sca")
            vec_files = glob.glob(f"{result_dir}/*.sca")
            
            if len(vec_files) and len(sca_files):
                print(f"Found results in {sub}")
                
                try:
                    datas.append(
                        extract_sim_data(
                            sca_files[0],
                            vec_files[0]
                        )
                    )
                except:
                    print("Failed")
                

    analyze_received_packets(datas)
