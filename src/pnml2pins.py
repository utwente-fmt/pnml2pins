'''
Copyright 2015 Formal Methods and Tools, University of Twente

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
'''
import xml.etree.ElementTree as etree
import sys, getopt, pickle

def parse_pnml(tree,ns):

    places=[]
    transitions=[]
    initial={}
    names={}
    safe_places=[]

    for node in tree.findall('.//{%s}place' % ns):
        place=node.get("id")
        places.append(place)
        initial[place]="0"
        names[place]=node.find('.//{%s}name' % ns).find('.//{%s}text' % ns).text
        for m in node.findall(".//{%s}initialMarking" % ns):
            for i in m.findall(".//{%s}text" % ns):
                initial[place]=i.text
    
    for node in tree.findall(".//{%s}transition" % ns):
        trans=node.get("id")
        transitions.append(trans)
        names[trans]=node.find('.//{%s}name' % ns).find('.//{%s}text' % ns).text

    sources = dict(map(lambda x: [x,[]],transitions))
    targets = dict(map(lambda x: [x,[]],transitions))

    for node in tree.findall(".//{%s}arc" % ns):
        src=node.get("source")
        trg=node.get("target")
        num=node.find(".//{%s}inscription" % ns)
        if (num != None):
            num=int(num.find(".//{%s}text" % ns).text)
        else:
            num=1
        if (src in places):
            sources[trg].append((src, num))
        else:
            targets[src].append((trg, num))
            
    for tool in tree.findall(".//{%s}toolspecific" % ns):
        for struct in tool.findall(".//{%s}structure" % ns):
            safe = struct.get("safe")
            if (safe == "true"):
                for stupid_xml in struct.findall(".//{%s}places" % ns):
                    split_places = stupid_xml.text.split()
                    for place in split_places:
                        safe_places.append(place)

    return (places,transitions,names,sources,targets,initial,safe_places)
    
def decl_header_dlopen():
    print("#include <string.h>")
    print("#include <stdbool.h>")
    print("#include <ltsmin/pins.h>")
    
def decl_sources_dlopen(transitions, sources, places):
    print("int* pnml_sources[%d] = {" % len(transitions))
    s = ""
    for transition in transitions:
        s += "    ((int[]) {%d, " % len(sources[transition])
        for (source,num) in sources[transition]:
            s += "%d, %d, " % (places.index(source), num)
        s += "}),\n"
    print(s)
    
    print("};")
    
def decl_targets_dlopen(transitions, targets, places):
    print("int* pnml_targets[%d] = {" % len(transitions))
    
    s = ""
    for transition in transitions:
        s += "    ((int[]) {%d, " % len(targets[transition])
        for (target,num) in targets[transition]:
            s += "%d, %d, " % (places.index(target), num)
        s += "}),\n"
    print(s)
    
    print("};")

def decl_sizes(places, transitions):
    print("const int pnml_vars = %d;" % len(places))
    print("const int pnml_groups = %d;" % len(transitions))

def decl_var_names(places, names):
    print("const char* pnml_var_names[%d] = {" % len(places))
    for place in places:
        print("    \"%s\"," % names[place])
    print("};")
    
def decl_group_names(transitions, names):
    print("const char* pnml_group_names[%d] = {" % len(transitions))
    for transition in transitions:
        print("    \"%s\"," % names[transition])
    print("};")
    
def decl_init_dlopen(places, initial):
    print("int pnml_initial_state[%d] = {" % len(places))
    
    s = ""
    for place in places:
        s += "    %s,\n" % initial[place]
    print("%s" % s, end="")
    
    print("};")
    
def decl_safe_places(places, safe_places):
    print("int pnml_safe_places[%d] = {" % len(places))
    
    for place in places:
        if place in safe_places:
            print("    1,")
        else:
            print("    0,")
    
    print("};")

def pnml_dlopen(places, transitions, names, sources, targets, initial, safe_places):
    decl_header_dlopen()
    print()
    decl_sources_dlopen(transitions, sources, places)
    print()
    decl_targets_dlopen(transitions, targets, places)
    print()
    decl_sizes(places, transitions)
    print()
    decl_var_names(places, names)
    print()
    decl_group_names(transitions, names)
    print()
    decl_init_dlopen(places, initial)
    print()
    decl_safe_places(places, safe_places)
    print()

def printusage(value):
    print("Usage: python3 pnml2pins.py [-h,--help] [input.pnml]")
    sys.exit(value)

def parseargs(argv):
    try:
        opts,args = getopt.getopt(argv,"h",["help"])
    except getopt.GetoptError:
        printusage(2)
    if (len(args)==1):
        input=args[0]
    elif (len(args)==0):
        input=sys.stdin
    else:
        printusage(2)
    return (input,opts)

def main():
    (input,opts) = parseargs(sys.argv[1:])
    tree=etree.parse(input).getroot()
    (places,transitions,names,sources,targets,initial,safe_places)=parse_pnml(tree,"http://www.pnml.org/version-2009/grammar/pnml")
    
    for opt,args in opts:
        if opt=="-h" or opt=="--help":
            printusage(0)
        
    
    pnml_dlopen(places,transitions,names,sources,targets,initial,safe_places)
            
    print("%d variables" % len(places), file=sys.stderr)
    print("%d groups" % len(transitions), file=sys.stderr)
    
'''
    with open('pn-sources', 'wb') as f:
        pickle.dump(sources, f)
        
    with open('pn-places', 'wb') as f:
        pickle.dump(places, f)
        
    with open('pn-names', 'wb') as f:
        pickle.dump(names, f)
'''

main()
