#pragma once
#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "dsp/basic_math_functions.h"
#include "dsp/statistics_functions.h"

#include <string>


using namespace arm_cmsis_stream;

extern "C" {
extern void node_softmax(float *in, size_t blockSize);
}

class KWSClassify: public StreamNode, public ContextSwitch
{
	static constexpr size_t nbLabels = 12;
	static constexpr size_t historySizeDefault = 4;
	static constexpr const char *labelsVec[nbLabels] = {
		"down",  "go",   "left", "no",  "off",       "on",
		"right", "stop", "up",   "yes", "_silence_", "_unknown_",
	};

      public:
	KWSClassify(EventQueue *queue, const KWSClassifyParams &params)
		: StreamNode(), ev0(queue), historySize_(params.historyLength)
	{
		history.resize(params.historyLength + 1);
		for (auto &v : history) {
			v.resize(nbLabels, 0.0f);
		}
	};

	int pause() final override
	{
		return 0;
	}

	int resume() final override
	{
		for (auto &v : history) {
			std::fill(v.begin(), v.end(), 0.0f);
		}
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
				LOG_INF("KWS Classify: %s\n", a);
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

	int computeClass(const float *t)
	{
		memcpy(buf, t, nbLabels * sizeof(float));
		// softmax
		node_softmax(buf, nbLabels);
		// add array to history
		for (int i = historySize_ - 1; i > 0; i--) {
			history[i] = std::move(history[i - 1]);
		}
		history[0] = std::vector<float>(buf, buf + nbLabels);

		memset(buf, 0, nbLabels * sizeof(float));
		for (const auto &v : history) {
			arm_add_f32(v.data(), buf, buf, nbLabels);
		}

		// find max
		uint32_t index;
		float res;
		arm_max_f32(buf, nbLabels, &res, &index);
		return index;
	}

	void processKWS(const TensorPtr<float> &t)
	{
		bool lockError;
		t.lock_shared(lockError, [this](const Tensor<float> &tensor) {
			int res = -1;
			const float *buf = tensor.buffer();
			res = computeClass(buf);
			this->sendLabel(res);
		});
		if (lockError) {
			LOG_ERR("KWSClassify: processKWS: lock error\n");
		}
	}

	void processConstantKWS(const TensorPtr<const float> &t)
	{
		bool lockError;
		t.lock_shared(lockError, [this](const Tensor<const float> &tensor) {
			int res = -1;
			const float *buf = tensor.buffer();
			res = computeClass(buf);
			sendLabel(res);
		});
		if (lockError) {
			LOG_ERR("KWSClassify: processConstantKWS: lock error\n");
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
				LOG_ERR("KWSClassify: processEvent: invalid event type\n");
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
	float buf[nbLabels];
	std::vector<std::vector<float>> history;
	EventOutput ev0;
	size_t historySize_;
};