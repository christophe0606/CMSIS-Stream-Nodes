#include "app_config.hpp"

extern "C" {
#include "app_params.h"
}

#include "scheduler_app.h"

#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "cstream_node.h"
#include "stream_init.hpp"
#include "stream_runtime_init.hpp"

#include <cstdint>
#include <cstdlib>

using namespace arm_cmsis_stream;

int currentNetwork = 0;
stream_execution_context_t contexts[NB_APPS];
EventQueue *queue_app[NB_APPS];
static HardwareParams hardwareParams;
static bool hardwareInitialized = false;

static void pause_scheduler_app(const stream_execution_context_t *context)
{
    for (int32_t nodeid = 0; nodeid < static_cast<int32_t>(context->nb_identified_nodes); nodeid++) {
        CStreamNode *cnode = static_cast<CStreamNode *>(context->get_node_by_id(nodeid));
        if ((cnode != nullptr) &&
            (cnode->obj != nullptr) &&
            (cnode->context_switch_intf != nullptr)) {
            cnode->context_switch_intf->pause(cnode->obj);
        }
    }
}

static void resume_scheduler_app(const stream_execution_context_t *context)
{
    for (int32_t nodeid = 0; nodeid < static_cast<int32_t>(context->nb_identified_nodes); nodeid++) {
        CStreamNode *cnode = static_cast<CStreamNode *>(context->get_node_by_id(nodeid));
        if ((cnode != nullptr) &&
            (cnode->obj != nullptr) &&
            (cnode->context_switch_intf != nullptr)) {
            cnode->context_switch_intf->resume(cnode->obj);
        }
    }
}

static void *get_app_node(int32_t nodeID)
{
    return static_cast<void *>(get_scheduler_app_node(nodeID));
}

static void handle_error(int32_t origin, int32_t error_code, int32_t info)
{
    (void)info;
    CMSISSTREAM_LOG_ERR("Error from origin %d with code %d\n", origin, error_code);
    stream_free_all(true);
    std::exit(1);
}

static bool application_handler(int src_node_id, void *data, Event &&evt)
{
    int network_id = static_cast<int>(reinterpret_cast<intptr_t>(data));
    if (evt.event_id == kError) {
        if (evt.wellFormed<int32_t, int32_t, int32_t>()) {
            evt.apply<int32_t, int32_t, int32_t>(&handle_error);
        }
    } else {
        CMSISSTREAM_LOG_DBG("Application handler received event %d from node %d in network %d\n",
                            evt.event_id, src_node_id, network_id);
    }
    return true;
}

int stream_configure_and_start()
{
    int err = hardware_params_init(&hardwareParams);
    if (err != 0) {
        CMSISSTREAM_LOG_ERR("Error initializing hardware parameters: %d\n", err);
        return err;
    }
    hardwareInitialized = true;
    app_params_set_hardware(&hardwareParams);

    err = stream_init_memory();
    if (err != 0) {
        CMSISSTREAM_LOG_ERR("Error initializing stream\n");
        goto error;
    }

    for (int network = 0; network < NB_APPS; network++) {
        queue_app[network] = stream_new_event_queue();

        if (queue_app[network] == nullptr) {
            CMSISSTREAM_LOG_ERR("Can't create CMSIS Stream Event Queue for network %d\n", network);
            goto error;
        }
        queue_app[network]->setHandler(reinterpret_cast<void *>(static_cast<intptr_t>(network)),
                                       application_handler);
    }

    err = init_scheduler_app(queue_app[0], &appParams);
    if (err != CG_SUCCESS) {
        CMSISSTREAM_LOG_ERR("Error: Failure during scheduler initialization for app graph.\n");
        goto error;
    }

    contexts[0] = stream_execution_context_t{
        scheduler_app,
        reset_fifos_scheduler_app,
        pause_scheduler_app,
        resume_scheduler_app,
        get_app_node,
        queue_app[0],
        STREAM_APP_NB_IDENTIFIED_NODES,
        STREAM_APP_SCHED_LEN};

    resume_scheduler_app(&contexts[currentNetwork]);
    if (!stream_start_threads(&contexts[currentNetwork])) {
        CMSISSTREAM_LOG_ERR("Error starting stream runtime threads\n");
        goto error;
    }

    return 0;

error:
    CMSISSTREAM_LOG_ERR("Fatal error in main, stopping execution\n");
    stream_free_all(false);
    return err != 0 ? err : -1;
}

void stream_free_all(bool callerIsRuntimeThread)
{
    stream_stop_threads(callerIsRuntimeThread);

    free_scheduler_app();

    for (int network = 0; network < NB_APPS; network++) {
        delete queue_app[network];
        queue_app[network] = nullptr;
    }

    stream_free_memory();

    if (hardwareInitialized) {
        hardware_params_uninit(&hardwareParams);
        hardwareInitialized = false;
    }
}
