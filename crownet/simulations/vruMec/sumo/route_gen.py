import xml.etree.ElementTree as ET
import random

NUM_PEDS = 50
GEN_VEH = True
NODE_EVERY = 0.01

tree = ET.parse('luitpoldpark_large/osm.net.xml')
root = tree.getroot()

def random_edge():

	edges = [
		'19683397#5',
		'19683871#3',
		'19683397#5',
		'559194509#0',
		'19683210#1',
		'19683798#3',
		'364097598#0',
		'-19683210#1',
		'82790717#0',
		'-82790717#4'
	]
	
	return random.choice(edges)

def random_Drive():

	edges = [
		'-88714905#7 -88714905#3 -88714905#2 -26632541#3',
		'376107908#0 376107908#1 88714905#1 88714905#3 19682357#1'
	]
	
	return random.choice(edges)

def main():
	print('<?xml version="1.0" encoding="UTF-8"?>')
	print('<routes xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.dlr.de/xsd/routes_file.xsd">')
	print('<vType id="ped_pedestrian" vClass="pedestrian"/>')
	print('<vType id="car_vh" accel="0.8" decel="4.5" sigma="0.5" length="5" maxSpeed="70"/>')
	
	for i in range(0, NUM_PEDS):

		print(f'<person id="ped{i}" depart="{"{:0.2f}".format(i * NODE_EVERY)}" type="ped_pedestrian">')
		print(f'<walk from="{random_edge()}" to="{random_edge()}"/>')
		print(f'</person>')
		
		if GEN_VEH:
			print(f'<vehicle id="veh{i}" type="car_vh" depart="{"{:0.3f}".format(i * NODE_EVERY + NODE_EVERY / 2)}{random.randrange(0,99)}">')
			print(f'<route edges="{random_Drive()}"/>')
			print(f'</vehicle>')
			
	print('</routes>')
		
if __name__ == '__main__':
	main()