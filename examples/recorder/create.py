from pathlib import Path
import sys

REPO_ROOT = Path(__file__).resolve().parents[2]
path_text = str(REPO_ROOT)
if REPO_ROOT.exists() and path_text not in sys.path:
    sys.path.insert(0, path_text)

from cmsis_stream.cg.scheduler import Graph,CType,SINT16
from examples.common.app import configure_app_from_args, mk_app
from nodes.generic import DebugSink, DebugSource  

config = configure_app_from_args()

the_graph = Graph()

sample_type = CType(SINT16)
block_size = 64
src_value = 2

src = DebugSource("src", sample_type, block_size,params={"value": ("APP_SRC_VALUE", sample_type)})
sink = DebugSink("sink", sample_type, block_size)

the_graph.connect(src.o, sink.i)

mk_app(
    the_graph,
    globals={
        "APP_BLOCK_SIZE": block_size,
        "APP_SRC_VALUE": src_value,
    },
    config=config,
)
