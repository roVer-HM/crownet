#!/usr/bin/env python3
import argparse
import xml.etree.ElementTree as ET


class Lane:
    def __init__(self, id, coords):
        self.id = id
        self.coords = coords

    def to_svg_path(self, bounds):
        def coordconv(x, y):
            return f'{x - bounds[0]},{(bounds[3] - bounds[1]) - (y - bounds[1])}'

        path = f'M {coordconv(self.coords[0][0], self.coords[0][1])}'
        for c in self.coords[1:]:
            path += f' L {coordconv(c[0], c[1])}'

        return path

    def to_path_element(self, bounds):
        return f"""
        <path
            style="stroke:#bbbbbb;stroke-width:2;fill:none;"
            d="{self.to_svg_path(bounds)}"
            id="sumo__{self.id}"/>
        """


class NetData:
    def __init__(self):
        self.y = None
        self.x = None
        self.h = None
        self.w = None
        self.net_offset_x = None
        self.net_offset_y = None
        self.lanes = []
        self.orig_bounds = None
        self.epsg = None

    def set_bounds(self, x, y, w, h, net_offset_x, net_offset_y, orig_bounds, epsg):
        self.w = w
        self.h = h
        self.x = x
        self.y = y
        self.net_offset_x = net_offset_x
        self.net_offset_y = net_offset_y
        self.orig_bounds = orig_bounds
        self.epsg = epsg

    def add_lane(self, lane):
        self.lanes.append(lane)

    def to_svg(self):
        svg = '<?xml version="1.0" encoding="UTF-8" standalone="no"?>'
        svg += f"""
        <svg
            width="{self.w - self.x}"
            height="{self.h - self.y}"
            viewBox="0 0 {self.w - self.x} {self.h - self.y}"
            data-epsg="{self.epsg}"
            data-offset-x="{self.net_offset_x}"
            data-offset-y="{self.net_offset_y}"
            data-sumo-origbounds="{self.orig_bounds}"
            version="1.1"
            id="svg5"
            xmlns="http://www.w3.org/2000/svg"
            xmlns:svg="http://www.w3.org/2000/svg">
            
        <g>
        """

        for lane in self.lanes:
            svg += lane.to_path_element((self.x, self.y, self.w, self.h))

        svg += """
        </g>
        </svg>
        """

        return svg


def main():
    net_data = NetData()

    parser = argparse.ArgumentParser(description='Generate SVG representation from sumo net files.')
    parser.add_argument('--net')
    parser.add_argument('--out')
    args = parser.parse_args()

    tree = ET.parse(args.net)

    net = tree.getroot()
    if net.tag != 'net':
        print('Expected toplevel tag "net"')

    for child in net:
        if child.tag == 'location':
            bounds = [float(b) for b in child.attrib['convBoundary'].split(',')]
            offsets = [float(b) for b in child.attrib['netOffset'].split(',')]

            net_data.set_bounds(
                bounds[0],
                bounds[1],
                bounds[2],
                bounds[3],
                offsets[0],
                offsets[1],
                child.attrib['origBoundary'],
                child.attrib['projParameter'],
            )

        if child.tag == 'edge':
            for lane in child:
                if lane.tag == 'lane':
                    if 'shape' in lane.attrib:
                        coords = [[float(cc) for cc in c.split(',')] for c in lane.attrib['shape'].split(' ')]
                        id = child.attrib['id']
                        net_data.add_lane(Lane(id, coords))

    with open(args.out, 'w') as f:
        f.write(net_data.to_svg())


if __name__ == '__main__':
    main()