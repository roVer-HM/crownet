.PHONY: svg
.PHONY: vadere

svg: mf_base.net.xml
	../sumo2svg.py --net mf_base.net.xml --out sumo.svg

vadere: vadere.svg
	../scenegen.py --svg vadere.svg --vadere vadere.scenario
