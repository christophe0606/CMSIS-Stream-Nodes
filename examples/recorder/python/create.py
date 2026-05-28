from pathlib import Path
import os
import subprocess
import sys

REPO_ROOT = Path(__file__).resolve().parents[3]
VIRTUAL_ENV = os.environ.get("VIRTUAL_ENV")
VENV_SITE_PACKAGES = (
    Path(VIRTUAL_ENV) / "Lib" / "site-packages"
    if VIRTUAL_ENV is not None
    else None
)

for path in (REPO_ROOT, VENV_SITE_PACKAGES):
    if path is None:
        continue
    path_text = str(path)
    if path.exists() and path_text not in sys.path:
        sys.path.insert(0, path_text)

from cmsis_stream.cg.scheduler import *  # noqa: E402,F403
from nodes.generic import DebugSink, DebugSource  # noqa: E402

cwd = Path.cwd()
target = (cwd / "../recorder_graph").resolve()

if not target.exists():
    print(f"The script must be launched from the python folder but you launched it from {cwd}.")

the_graph = Graph()

sample_type = CType(SINT16)
block_size = 64

src = DebugSource("src", sample_type, block_size)
sink = DebugSink("sink", sample_type, block_size)

the_graph.connect(src.o, sink.i)

conf = Configuration()
conf.CMSISDSP = False
conf.asynchronous = False
conf.horizontal = True
conf.nodeIdentification = True
conf.schedName = "scheduler_recorder"
conf.schedulerCFileName = "scheduler_recorder"
conf.memoryOptimization = True
conf.appConfigCName = "app_config.hpp"
conf.appNodesCName = "AppNodes_recorder.hpp"
conf.prefix = "stream_recorder_"

scheduling = the_graph.computeSchedule(config=conf)

print("Schedule length = %d" % scheduling.scheduleLength)
print("Memory usage %d bytes" % scheduling.memory)

scheduling.ccode("../recorder_graph", conf)
scheduling.genJsonIdentification("../recorder_graph/json", conf)
scheduling.genJsonSelectorsInit("../recorder_graph/json", conf)

with open("../recorder_graph/recorder.dot", "w") as f:
    scheduling.graphviz(f)

try:
    subprocess.run(
        ["dot", "-Tpng", "../recorder_graph/recorder.dot", "-o", "../recorder_graph/recorder.png"],
        check=False,
    )
except FileNotFoundError:
    pass
