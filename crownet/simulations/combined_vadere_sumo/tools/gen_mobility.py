#!/usr/bin/env python3

flows = [
    # route, per spawn, total, delay (s), is Person
    # (
    #    # East to north
    #    '-31344484 -33487028#4 -33487028#3 -33487028#2 -33487028#1 24039836#1 24039836#4 24039836#5 24039836#6 24039835#1 193703019#0',
    #    5,
    #    20,
    #    40,
    #    True
    # ),
    # (
    #    # North to east
    #    '193703019#0 24039835#1 24039836#6 24039836#5 24039836#4 24039836#1 -33487028#1 -33487028#2 -33487028#3 -33487028#4 -31344484',
    #    5,
    #    20,
    #    40,
    #    True
    # ),
    (
      # Car traffic East to north
      '86414058 -31344484 -33487028#4 -33487028#3 -33487028#2 -33487028#1 -155548874 351235033 332176203 332176200',
      1,
      20,
      20,
      False  
    ),
    (
      # Car traffic North to south
      '127977585 93607081 265463850#0 157964860#0 151542345#1 151542345#2',
      1,
      20,
      20,
      False  
    )
]

def main():
    # depart, route, is_person
    nodes = []
    
    stat = {
        'pno': 0,
        'vno': 0
    }
    
    for id, flow in enumerate(flows):
        n_pers = 0
        start_t = 0
        
        while n_pers < flow[2]:
            
            t_offset = 0
            for i in range(0, flow[1]):
                
                t_offset += 0.01
                nodes.append((
                    start_t + t_offset + (id * 0.001),
                    flow[0],
                    flow[4]
                ))
                n_pers += 1
                
                if (flow[4]):
                    stat['pno'] += 1
                else:
                    stat['vno'] += 1
                
            start_t += flow[3]
            
    nodes.sort(key=lambda p : p[0])
            
                
    for id, n in enumerate(nodes):
        
        if (n[2]):
            print(
                f"""
                <person id="{id}" depart="{n[0]}" type="ptype1">
                    <walk edges="{n[1]}"/>
                </person>
                """
            )
        else:
           print(
                f"""
                <vehicle id="{id}" type="type1" depart="{n[0]}">
                    <route edges="{n[1]}"/>
                </vehicle>
                """
            )
    
    print(stat)

if __name__ == '__main__':
    main()

