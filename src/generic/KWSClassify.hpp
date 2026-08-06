#pragma once
#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "common/KWSClassifyCompute.hpp"

#include <string>
#include <cstdio>


class KWSClassify: public StreamNode, public ContextSwitch
{
	static constexpr size_t nbLabels = 12;
	static constexpr const char *labelsVec[nbLabels] = {
		"down",  "go",   "left", "no",  "off",       "on",
		"right", "stop", "up",   "yes", "_silence_", "_unknown_",
	};

public:
	KWSClassify(EventQueue *queue, const KWSClassifyParams &params)
		: StreamNode(), ev0(queue), compute_(params.historyLength)
	{
	};

	int pause() final override
	{
		return 0;
	}

	int resume() final override
	{
		compute_.resume();
		lastRec = 11;
		return 0;
	}

	

	virtual ~KWSClassify()
	{
	}

	void sendLabel(int c)
	{
		if (c < 0) {
			return;
		}

		uint32_t label_idx = static_cast<uint32_t>(c);
		if (label_idx < nbLabels - 2) {
			const char *a = labelsVec[label_idx];
			if (label_idx != lastRec) {
				printf("KWS Classify: %s\n", a);
			}
			ev0.sendSync(kNormalPriority, kValue,
					     (uint32_t)label_idx); // Send the event to the
								   // subscribed nodes
		}
		if (label_idx != lastRec) 
		{
			lastRec = label_idx;
		}
	}

	void processKWS(const TensorPtr<float> &t)
	{
		bool lockError;
		t.lock_shared(lockError, [this](const Tensor<float> &tensor) {
			int res = -1;
			const float *buf = tensor.buffer();
			res = this->compute_(buf);
			this->sendLabel(res);
		});
		if (lockError) {
			CMSISSTREAM_LOG_ERR("KWSClassify: processKWS: lock error\n");
		}
	}

	void processConstantKWS(const TensorPtr<const float> &t)
	{
		bool lockError;
		t.lock_shared(lockError, [this](const Tensor<const float> &tensor) {
			int res = -1;
			const float *buf = tensor.buffer();
			res = this->compute_(buf);
			this->sendLabel(res);
		});
		if (lockError) {
			CMSISSTREAM_LOG_ERR("KWSClassify: processConstantKWS: lock error\n");
		}
	}

	cg_status processEvent(int dstPort, Event &&evt) final override
	{
		if (evt.event_id == kValue) {
			if (evt.wellFormed<TensorPtr<float>>()) {
				evt.apply<TensorPtr<float>>(&KWSClassify::processKWS, *this);
			}
			else if (evt.wellFormed<TensorPtr<const float>>()) {
				evt.apply<TensorPtr<const float>>(&KWSClassify::processConstantKWS,
								  *this);
			}
			else
			{
				CMSISSTREAM_LOG_ERR("KWSClassify: processEvent: invalid event type\n");
				return CG_OS_ERROR;
			}
		}
		return CG_SUCCESS;
	}

	void subscribe(int outputPort, StreamNode &dst, int dstPort) final override
	{
		ev0.subscribe(dst, dstPort);
	}

protected:
	uint32_t lastRec{11};
	EventOutput ev0;
	KWSClassifyCompute compute_;

};