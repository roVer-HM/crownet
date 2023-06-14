from crownetutils.analysis.common import Simulation
from crownetutils.analysis.omnetpp import OppAnalysis, HdfExtractor
from crownetutils.analysis.plot.app_misc import PlotAppMisc
from crownetutils.analysis.plot.enb import PlotEnb


def build_hdf(sim: Simulation):
    _, builder, _ = OppAnalysis.builder_from_output_folder(data_root=sim.data_root)
    builder.only_selected_cells(True)
    builder.build(override_hdf=False)

    OppAnalysis.append_err_measures_to_hdf(sim)

    HdfExtractor.extract_rvcd_statistics(sim.path("rcvd_stats.h5"), sim.sql)

    HdfExtractor.extract_packet_loss(
        sim.path("pkt_loss.h5"), "map", sim.sql, sim.sql.m_map()
    )
    HdfExtractor.extract_packet_loss(
        sim.path("pkt_loss.h5"), "beacon", sim.sql, sim.sql.m_beacon()
    )


def main():
    sim = Simulation(
        data_root="/mnt/data1tb/results/multi_enb/test_run_5000s/",
        label="mulit_enb",
    )
    build_hdf(sim)


if __name__ == "__main__":
    main()
