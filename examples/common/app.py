from cmsis_stream.cg.scheduler import Configuration
import argparse
from dataclasses import dataclass
import math
import subprocess
from pathlib import Path

RUNNERS = ("zephyr", "posix", "cmsis")
BOARDS = ("AlifE7", "Linux", "Mac", "Windows","fvp_cs300")
REPO_ROOT = Path(__file__).resolve().parents[2]
COMMON_APP_GRAPH_DIR = REPO_ROOT / "runner_common" / "app_graph"

@dataclass(frozen=True)
class AppConfig:
    runner: str = "posix"
    board: str | None = None

_app_config = AppConfig()

def parse_app_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", choices=RUNNERS, default="posix")
    parser.add_argument("--board", choices=BOARDS, default=None)
    return parser.parse_args()

def set_app_config(config):
    global _app_config
    _app_config = config
    return _app_config

def get_app_config():
    return _app_config

def configure_app(runner="posix", board=None):
    if runner not in RUNNERS:
        raise ValueError(f"Unsupported runner '{runner}'")
    if board is not None and board not in BOARDS:
        raise ValueError(f"Unsupported board '{board}'")
    return set_app_config(AppConfig(runner=runner, board=board))

def configure_app_from_args():
    args = parse_app_args()
    return configure_app(runner=args.runner, board=args.board)

def mk_app_from_args(the_graph, params=None, globals=None):
    config = configure_app_from_args()
    return mk_app(the_graph, params=params, globals=globals, config=config)

def _runner_dir(runner):
    if runner not in RUNNERS:
        raise ValueError(f"Unsupported runner '{runner}'")
    return f"{runner}_runner"

def _board_name(board):
    return board if board is not None else "default"

def _is_default_node(node):
    return not hasattr(node, "folder")

def _is_typed_param(value):
    return (
        isinstance(value, tuple)
        and len(value) == 2
        and hasattr(value[1], "ctype")
    )

def _is_valid_c_identifier(name):
    return (
        isinstance(name, str)
        and len(name) > 0
        and (name[0].isalpha() or name[0] == "_")
        and all(c.isalnum() or c == "_" for c in name)
    )

def _infer_param_type(name, value):
    if isinstance(value, bool):
        return "uint8_t", 1 if value else 0

    if isinstance(value, int):
        if value < -(2**31) or value > 2**31 - 1:
            print(f"Warning: parameter '{name}' value {value} does not fit in int32_t")
        return "int32_t", value

    if isinstance(value, float):
        if math.isfinite(value) and abs(value) > 3.4028234663852886e38:
            print(f"Warning: parameter '{name}' value {value} does not fit in float")
        return "float", value

    raise TypeError(f"Unsupported parameter value for '{name}': {value!r}")

def _param_decl_and_value(name, value):
    if not _is_valid_c_identifier(name):
        raise ValueError(f"Invalid C field name for parameter: {name!r}")

    if _is_typed_param(value):
        literal, the_type = value
        if isinstance(literal, bool):
            literal = 1 if literal else 0
        return the_type.ctype, literal

    return _infer_param_type(name, value)

def _node_name(node):
    return getattr(node, "nodeName", getattr(node, "name", None))

def _node_struct_name(type_name):
    return f"{type_name}Params"

def _node_type_name(node):
    return getattr(node, "typeName", node.__class__.__name__)

def _node_needs_hardware(node):
    return bool(getattr(node, "needsHardware", False))

def _node_params(node):
    params = getattr(node, "params", {})
    if params is None:
        return {}
    if not isinstance(params, dict):
        name = _node_name(node)
        raise TypeError(f"Parameters for node '{name}' must be a dictionary")
    return params

def _target_dir(runner=None):
    return str(COMMON_APP_GRAPH_DIR)

def _hardware_target(runner, board):
    return f"{runner}/{_board_name(board)}"

def _format_c_literal(value, c_type):
    if isinstance(value, bool):
        value = 1 if value else 0

    if isinstance(value, int):
        return str(value)

    if isinstance(value, float):
        if math.isnan(value):
            return "NAN"
        if math.isinf(value):
            return "INFINITY" if value > 0 else "-INFINITY"

        literal = repr(value)
        if c_type in ("float", "float32_t", "float16_t") and "e" not in literal.lower() and "." not in literal:
            literal += ".0"
        if c_type in ("float", "float32_t", "float16_t"):
            literal += "f"
        return literal

    return str(value)

def _format_global_define(name, value):
    if not _is_valid_c_identifier(name):
        raise ValueError(f"Invalid C macro name for global define: {name!r}")

    if value is None:
        return None

    if isinstance(value, str):
        return value

    c_type, literal = _param_decl_and_value(name, value)
    return _format_c_literal(literal, c_type)

def _global_defines(globals):
    if globals is None:
        return []
    if not isinstance(globals, dict):
        raise TypeError("Global defines must be a dictionary")
    return [(name, _format_global_define(name, value)) for name, value in globals.items()]

def _gen_params(params, scheduling, runner, globals=None):
    node_params = {}
    params = params or {}
    global_defines = _global_defines(globals)

    for node_name, node_values in params.items():
        if not _is_valid_c_identifier(node_name):
            raise ValueError(f"Invalid C field name for AppParams: {node_name!r}")
        if node_values is None:
            node_values = {}
        if not isinstance(node_values, dict):
            raise TypeError(f"Parameters for node '{node_name}' must be a dictionary")
        node_params[node_name] = {
            "type_name": None,
            "needs_hardware": False,
            "fields": [],
        }
        for param_name, value in node_values.items():
            c_type, literal = _param_decl_and_value(param_name, value)
            node_params[node_name]["fields"].append((param_name, c_type, literal))

    for node in scheduling.allNodes:
        if _is_default_node(node):
            continue
        name = _node_name(node)
        if name is None:
            continue
        if not _is_valid_c_identifier(name):
            raise ValueError(f"Invalid C field name for AppParams: {name!r}")
        type_name = _node_type_name(node)
        if not _is_valid_c_identifier(type_name):
            raise ValueError(f"Invalid C parameter type name for node '{name}': {type_name!r}")
        node_local_params = _node_params(node)
        if _node_needs_hardware(node) or name in node_params or node_local_params:
            node_params.setdefault(name, {"type_name": None, "needs_hardware": False, "fields": []})
            if node_params[name]["type_name"] is not None and node_params[name]["type_name"] != type_name:
                raise ValueError(
                    f"Conflicting typeName for node '{name}': "
                    f"{node_params[name]['type_name']!r} and {type_name!r}"
                )
            node_params[name]["type_name"] = type_name
            node_params[name]["needs_hardware"] = _node_needs_hardware(node)
            existing = {field_name for field_name, _, _ in node_params[name]["fields"]}
            for param_name, value in node_local_params.items():
                if param_name in existing:
                    continue
                c_type, literal = _param_decl_and_value(param_name, value)
                node_params[name]["fields"].append((param_name, c_type, literal))

    app_fields = list(node_params.items())
    for node_name, node_data in app_fields:
        if node_name != "reserved" and node_data["type_name"] is None:
            node_data["type_name"] = node_name

    struct_defs = {}
    for node_name, node_data in app_fields:
        if node_name == "reserved":
            continue
        struct_name = _node_struct_name(node_data["type_name"])
        signature = (
            node_data["needs_hardware"],
            tuple((param_name, c_type) for param_name, c_type, _ in node_data["fields"]),
        )
        if struct_name in struct_defs and struct_defs[struct_name][0] != signature:
            raise ValueError(f"Conflicting parameter layouts for generated type '{struct_name}'")
        struct_defs.setdefault(struct_name, (signature, node_data))

    if not app_fields:
        app_fields.append(("reserved", {"needs_hardware": False, "fields": [("uint8_t", 0)]}))

    target = _target_dir(runner)

    with open(f"{target}/app_params.h","w") as f:
        print("// This file is automatically generated by app.py. Do not edit it manually.",file=f)
        print("#ifndef APP_PARAMS_H",file=f)
        print("#define APP_PARAMS_H",file=f)
        print("",file=f)
        print("#include <stdint.h>",file=f)
        print("#include \"hardware_params.h\"",file=f)
        print("",file=f)
        for name, value in global_defines:
            if value is None:
                print(f"#define {name}",file=f)
            else:
                print(f"#define {name} {value}",file=f)
        if global_defines:
            print("",file=f)
        for struct_name, (_, node_data) in struct_defs.items():
            print("typedef struct {",file=f)
            if node_data["needs_hardware"]:
                print("    HardwareParams hw_;",file=f)
            for param_name, c_type, _ in node_data["fields"]:
                print(f"    {c_type} {param_name};",file=f)
            if not node_data["needs_hardware"] and not node_data["fields"]:
                print("    uint8_t reserved;",file=f)
            print(f"}} {struct_name};",file=f)
            print("",file=f)
        print("typedef struct {",file=f)
        for node_name, node_data in app_fields:
            if node_name == "reserved":
                print("    uint8_t reserved;",file=f)
            else:
                print(f"    {_node_struct_name(node_data['type_name'])} {node_name};",file=f)
        print("} AppParams;",file=f)
        print("",file=f)
        print("extern AppParams appParams;",file=f)
        print("void app_params_set_hardware(const HardwareParams *params);",file=f)
        print("",file=f)
        print("#endif",file=f)

    with open(f"{target}/app_params.c","w") as f:
        print("// This file is automatically generated by app.py. Do not edit it manually.",file=f)
        print("#include \"app_params.h\"",file=f)
        print("#include <math.h>",file=f)
        print("",file=f)
        print("AppParams appParams = {",file=f)
        for node_name, node_data in app_fields:
            if node_name == "reserved":
                print("    .reserved = 0,",file=f)
                continue
            print(f"    .{node_name} = {{",file=f)
            for param_name, c_type, literal in node_data["fields"]:
                print(f"        .{param_name} = {_format_c_literal(literal, c_type)},",file=f)
            if not node_data["needs_hardware"] and not node_data["fields"]:
                print("        .reserved = 0,",file=f)
            print("    },",file=f)
        print("};",file=f)
        print("",file=f)
        print("void app_params_set_hardware(const HardwareParams *params)",file=f)
        print("{",file=f)
        print("    if (params == 0) {",file=f)
        print("        return;",file=f)
        print("    }",file=f)
        for node_name, node_data in app_fields:
            if node_name != "reserved" and node_data["needs_hardware"]:
                print(f"    appParams.{node_name}.hw_ = *params;",file=f)
        print("}",file=f)
 

def _gen_node_includes(scheduling, runner):
    with open(f"{_target_dir(runner)}/AppNodes.hpp","w") as f:
        print("// This file is automatically generated by app.py. Do not edit it manually.",file=f)
        print("#pragma once",file=f)
        # Get all nodes from this demo and ignore default nodes coming from CMSIS Stream package
        for n in scheduling.allNodes:
            if not _is_default_node(n):
                print(f"#include \"{n.folder}/{n.typeName}.hpp\"",file=f)

def _gen_build_config(runner, board):
    with open(f"{_target_dir(runner)}/board_config.cmake","w") as f:
        print("# This file is automatically generated by app.py. Do not edit it manually.",file=f)
        print(f"set(APP_GENERATED_RUNNER \"{runner}\")",file=f)
        print(f"set(APP_GENERATED_BOARD \"{_board_name(board)}\")",file=f)
        print(f"set(APP_GENERATED_HARDWARE_TARGET \"{_hardware_target(runner, board)}\")",file=f)

def mk_app(the_graph, params=None, globals=None, config=None, runner=None, board=None):
    if config is None:
        if runner is None and board is None:
            config = get_app_config()
        else:
            config = configure_app(
                runner=runner if runner is not None else get_app_config().runner,
                board=board,
            )
    runner = config.runner
    board = config.board

    target = COMMON_APP_GRAPH_DIR

    if not target.exists():
       target.mkdir(parents=True, exist_ok=True)
    (target / "json").mkdir(parents=True, exist_ok=True)
 
    conf = Configuration()
    conf.CMSISDSP = False
    conf.asynchronous = False
    conf.horizontal = True
    conf.nodeIdentification = True
    conf.schedName = "scheduler_app"
    conf.schedulerCFileName = "scheduler_app"
    conf.memoryOptimization = True
    conf.appConfigCName = "app_config.hpp"
    conf.appNodesCName = "AppNodes.hpp"
    conf.cOptionalInitArgs = [f"AppParams *params"]
    conf.prefix = "stream_app_"
    
    scheduling = the_graph.computeSchedule(config=conf)
    
    print("Schedule length = %d" % scheduling.scheduleLength)
    print("Memory usage %d bytes" % scheduling.memory)
    
    scheduling.ccode(_target_dir(runner), conf)
    scheduling.genJsonIdentification(f"{_target_dir(runner)}/json", conf)
    scheduling.genJsonSelectorsInit(f"{_target_dir(runner)}/json", conf)

    _gen_node_includes(scheduling, runner)
    _gen_params(params, scheduling, runner, globals=globals)
    _gen_build_config(runner, board)
    
    with open(f"{_target_dir(runner)}/app.dot", "w") as f:
        scheduling.graphviz(f)
    
    try:
        subprocess.run(
            ["dot", "-Tpng", f"{_target_dir(runner)}/app.dot", "-o", f"{_target_dir(runner)}/app.png"],
            check=False,
        )
    except FileNotFoundError:
        pass

    print(f"Runner = {runner}")
    print(f"Board = {_board_name(board)}")
    print(f"Hardware target = {_hardware_target(runner, board)}")
    
