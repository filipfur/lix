import bpy
        
def fileNameOnly(pth):
    return pth[pth.rfind("/")+1:pth.rfind(".")]
        
def generate_header_file():
    
    linkedObjects = {}
    collectionNodes = []

    fname = fileNameOnly(bpy.data.filepath)
    for obj in bpy.data.objects:
        icol = obj.instance_collection
        if icol is None:
            if obj.data.library is None:
                collectionNodes.append((obj, bpy.data.filepath, obj))
                linkedObjects[bpy.data.filepath] = obj
                #print("REGULAR: " + obj.name + " " + obj.type + " " + str(obj.location))
        else:
            found = None
            for o in icol.objects:
                if o.type == "MESH":
                    collectionNodes.append((obj, icol.library.filepath, o))
                    linkedObjects[icol.library.filepath] = o
                    found = o
                    break
            if found is None:
                print(f"NONE FOUND: {icol.name}")
                exit(1)
    
    with open(bpy.path.abspath(f"//{fname}.h"), "w") as f:
        f.write("#pragma once\n")
        for fn in [ fileNameOnly(pth) for pth in linkedObjects ]:
            f.write(f"#include \"gen/objects/{fn}.h\"\n")
        f.write(f"namespace assets::levels::{fname} {{\n\n    const gltf::Node nodes[] = {{")
        delim = ""
        for obj, pth, o in collectionNodes:
            fname = fileNameOnly(pth)
            
            t = obj.location
            r = obj.rotation_quaternion if obj.rotation_mode == 'QUATERNION' else obj.rotation_euler.to_quaternion()
            s = obj.scale
            
            meshName = o.data.name.replace(".", "_")
            
            f.write(delim + f"""
        {{
            \"\",
            &assets::objects::{fname}::{meshName}_mesh,
            {{{t.x}f, {t.z}f, {-t.y}f}},
            {{{r.w}f, {r.x}f, {r.z}f, {-r.y}f}},
            {{{s.x}f, {s.z}f, {s.y}f}},
            nullptr,
            nullptr,
            0
        }}""")
            delim = ","
        f.write("\n    };\n}\n")

generate_header_file()