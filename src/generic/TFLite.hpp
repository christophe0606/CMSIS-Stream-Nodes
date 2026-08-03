#pragma once

#include "EventQueue.hpp"
#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include "app_params.h"
#include "network_sections.h"

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/schema/schema_utils.h"

#include <inttypes.h>
#include <variant>


static uint8_t tensorArena[CONFIG_ACTIVATION_BUF_SZ] ACTIVATION_BUF_ATTRIBUTE;

using namespace arm_cmsis_stream;

class TFLite : public StreamNode
{
  public:
    enum selector {selAck=0};
    TFLite(EventQueue *queue, const KWSParams &params,uint32_t nbOutputs=1)
        : StreamNode(),
          tensorArenaAddr_(tensorArena),
          tensorArenaSize_(sizeof(tensorArena)),initErrorOccured(false),
          ev(1 + nbOutputs, EventOutput(queue))
    {

        if (params.hw_.model_weights == nullptr || params.hw_.model_size == 0)
        {
            CMSISSTREAM_LOG_ERR("TFLite: Invalid model address or size\n");
            initErrorOccured = true;
            return;
        }
        this->m_pModel = ::tflite::GetModel(params.hw_.model_weights);

        if (this->m_pModel->version() != TFLITE_SCHEMA_VERSION)
        {
            CMSISSTREAM_LOG_ERR("TFLite: Model's schema version %" PRIu32 " is not equal "
                        "to supported version %d.",
                        this->m_pModel->version(),
                        TFLITE_SCHEMA_VERSION);
            initErrorOccured = true;
            return;
        }

        this->m_modelAddr = params.hw_.model_weights;
        this->m_modelSize = params.hw_.model_size;
        this->m_pAllocator = nullptr;

        
    }

    virtual ~TFLite()
    {
    }

    cg_status init() final override
    {

        if (initErrorOccured)
        {
            return CG_INIT_FAILURE;
        }
        if (this->m_modelAddr == nullptr || this->m_modelSize == 0)
        {
            CMSISSTREAM_LOG_ERR("TFLite: Invalid model address or size\n");
            return CG_INIT_FAILURE;
        }
        
        bool ok = this->enlistOperations();
        if (!ok || initErrorOccured)
        {
            return CG_INIT_FAILURE;
        }

        if (!this->m_pAllocator)
        {
            /* Create an allocator instance */
            CMSISSTREAM_LOG_DBG("Creating allocator using tensor arena at 0x%p\n", tensorArenaAddr_);

            this->m_pAllocator = tflite::MicroAllocator::Create(tensorArenaAddr_, tensorArenaSize_);

            if (!this->m_pAllocator)
            {
                CMSISSTREAM_LOG_ERR("TFLite: Failed to create allocator\n");
                return CG_INIT_FAILURE;
            }
            CMSISSTREAM_LOG_DBG("TFLite: Created new allocator @ 0x%p\n", this->m_pAllocator);
        }
        else
        {
            CMSISSTREAM_LOG_DBG("Using existing allocator @ 0x%p\n", this->m_pAllocator);
        }

        this->m_pInterpreter = std::make_unique<tflite::MicroInterpreter>(
            this->m_pModel, this->GetOpResolver(), this->m_pAllocator);

        if (!this->m_pInterpreter)
        {
            CMSISSTREAM_LOG_ERR("TFLite: Failed to allocate interpreter\n");
            return CG_INIT_FAILURE;
        }

        /* Allocate memory from the tensor_arena for the model's tensors. */
        CMSISSTREAM_LOG_DBG("TFLite: Allocating tensors\n");
        TfLiteStatus allocate_status = this->m_pInterpreter->AllocateTensors();

        if (allocate_status != kTfLiteOk)
        {
            CMSISSTREAM_LOG_ERR("tensor allocation failed!\n");
            return CG_INIT_FAILURE;
        }

        this->m_input.resize(this->GetNumInputs());
        for (size_t inIndex = 0; inIndex < this->GetNumInputs(); inIndex++)
            this->m_input[inIndex] = this->m_pInterpreter->input(inIndex);

        this->m_output.resize(this->GetNumOutputs());
        for (size_t outIndex = 0; outIndex < this->GetNumOutputs(); outIndex++)
            this->m_output[outIndex] = this->m_pInterpreter->output(outIndex);

        if (this->m_input.empty() || this->m_output.empty())
        {
            CMSISSTREAM_LOG_ERR("TFLite: Failed to get tensors\n");
            return CG_INIT_FAILURE;
        }
        else
        {
            this->m_type = this->m_input[0]->type; /* Input 0 should be the main input */

            /* Clear the input & output tensors */
            for (size_t inIndex = 0; inIndex < this->GetNumInputs(); inIndex++)
            {
                std::memset(this->m_input[inIndex]->data.data, 0, this->m_input[inIndex]->bytes);
            }
            for (size_t outIndex = 0; outIndex < this->GetNumOutputs(); outIndex++)
            {
                std::memset(this->m_output[outIndex]->data.data, 0, this->m_output[outIndex]->bytes);
            }

            // this->LogInterpreterInfo();
        }

        
        if (!ev.empty())
        {
            ev[0].sendSync(kNormalPriority, globalID(selAck)); // Notify that initialization is done
        }
        else {
            CMSISSTREAM_LOG_ERR("TFLite: Failed to create event outputs\n");
        }

        this->m_inited = true;

        
        return (CG_SUCCESS);
    }

    void getOutputQuantizationParams(TfLiteTensor *t, float &scale, int &offset)
    {
        if (kTfLiteAffineQuantization == t->quantization.type)
        {
            auto *quantParams = (TfLiteAffineQuantization *)(t->quantization.params);
            if (quantParams && 0 == quantParams->quantized_dimension)
            {
                if (quantParams->scale->size)
                {
                    scale = quantParams->scale->data[0];
                }
                if (quantParams->zero_point->size)
                {
                    offset = quantParams->zero_point->data[0];
                }
            }
            else if (t->params.scale != 0.0f)
            {
                /* Legacy tensorflow quantisation parameters */
                scale = t->params.scale;
                offset = t->params.zero_point;
            }
        }
    }

    void sendTensor(int dstPort, TfLiteTensor *t)
    {
        if ((uint32_t)dstPort >= this->GetNumOutputs())
            return;
        //size_t bytes = t->bytes;
        size_t elements = 1;
        cg_tensor_dims_t dims;
        for (int i = 0; i < t->dims->size; i++)
        {
            dims[i] = t->dims->data[i];
            elements *= t->dims->data[i];
        }

        UniquePtr<float> tensorData(elements);
        float scale = 1.0;
        int offset = 0;

        getOutputQuantizationParams(t, scale, offset);

        switch (t->type)
        {
        case kTfLiteInt8:
        {
            int8_t *tensor_buffer = (int8_t *)t->data.raw;
            for (size_t i = 0; i < elements; ++i)
            {
                tensorData[i] = scale *
                                (static_cast<float>(tensor_buffer[i]) - offset);
            }

            break;
        }
        case kTfLiteUInt8:
        {
            uint8_t *tensor_buffer = (uint8_t *)t->data.raw;
            for (size_t i = 0; i < elements; ++i)
            {
                tensorData[i] = scale *
                                (static_cast<float>(tensor_buffer[i]) - offset);
            }

            break;
        }
        default:
        {
            CMSISSTREAM_LOG_ERR("TFLite: Unsupported data type %d\n", t->type);
            return;
        }
        }

        TensorPtr<float> tensor = TensorPtr<float>::create_with((uint8_t)t->dims->size,
                                                                std::move(dims),
                                                                std::move(tensorData));
        // Synchronous send because it is sent from the context
        // of the processEvent call
        // and we do not care about the priority here
        if (ev.size() > (size_t)(dstPort + 1))
        {
            ev[dstPort + 1].sendSync(kNormalPriority, kValue, std::move(tensor)); // Send the event to the subscribed nodes
        }
    }

    // Selector array is initialized in the derived class
    virtual int globalID(int localID) = 0;

    void tryInference()
    {
        uint32_t nb = this->GetNumInputs();
        // If all inputs have been received
        if ((int)inputReceived == ((1 << nb) - 1))
        {
            inputReceived = 0;
            TfLiteStatus invoke_status = this->m_pInterpreter->Invoke();
            if (invoke_status != kTfLiteOk)
            {
                CMSISSTREAM_LOG_ERR("TFLite: Invoke failed on model\n");
                return;
            }
            // Output tensors are ready
            for (size_t outIndex = 0; outIndex < this->GetNumOutputs(); outIndex++)
            {
                TfLiteTensor *outputTensor = this->m_output.at(outIndex);

                sendTensor(outIndex, outputTensor);
            }
            // Send acknowledge event to the producer
            if (!ev.empty())
            {
                ev[0].sendSync(kNormalPriority, globalID(selAck));
            }
        }
    }

    template <typename T>
    void convertReceivedF32Tensor(int dstPort, TensorPtr<T> &&input)
    {

        if ((size_t)dstPort >= this->GetNumInputs())
            return;

        TfLiteTensor *inputTensor = this->m_input.at(dstPort);

        bool lockError;
        // Input tensor
        input.lock_shared(lockError, [inputTensor, dstPort, this](const Tensor<T> &tensor)
                          {
                              using pointee_t = std::remove_const_t<T>;
                              const pointee_t *buf = tensor.buffer();
                              size_t bytes = tensor.size() * sizeof(T);
                              switch (inputTensor->type)
                              {
                              case kTfLiteFloat32:
                                  if (bytes == inputTensor->bytes)
                                  {
                                      memcpy(inputTensor->data.raw, buf, tensor.size() * sizeof(T));
                                      inputReceived |= (1 << dstPort);
                                  }
                                  break;
                              case kTfLiteInt8:
                              {
                                  TfLiteQuantization quant = inputTensor->quantization;
                                  if (kTfLiteAffineQuantization == quant.type)
                                  {
                                      auto *quantParams = (TfLiteAffineQuantization *)quant.params;
                                      const float quantScale = quantParams->scale->data[0];
                                      const int quantOffset = quantParams->zero_point->data[0];
                                      for (size_t i = 0; i < tensor.size(); i++)
                                      {
                                          float val = (float)buf[i];
                                          int8_t quantVal = (int8_t)(val / quantScale) + quantOffset;
                                          if (quantVal > 127)
                                              quantVal = 127;
                                          else if (quantVal < -128)
                                              quantVal = -128;
                                          ((int8_t *)inputTensor->data.data)[i] = quantVal;
                                      }
                                  }
                                  else
                                  {
                                      for (size_t i = 0; i < tensor.size(); i++)
                                      {
                                          float val = (float)buf[i];
                                          if (val > 127)
                                              val = 127;
                                          else if (val < -128)
                                              val = -128;
                                          ((int8_t *)inputTensor->data.data)[i] = (int8_t)val;
                                      }
                                  }
                                  inputReceived |= (1 << dstPort);
                                }
                                  break;
                              default:
                                  CMSISSTREAM_LOG_ERR("TFLite: Unsupported tensor input data type %d\n", inputTensor->type);
                                  return;
                              } });

        tryInference();
    }

    template <typename T>
    void convertReceivedInt8Tensor(int dstPort, TensorPtr<T> &&input)
    {

        if ((size_t)dstPort >= this->GetNumInputs())
            return;

        

        TfLiteTensor *inputTensor = this->m_input.at(dstPort);
        bool lockError;
        // Input tensor
        input.lock_shared(lockError, [inputTensor, dstPort,this](const Tensor<T> &tensor)
                          {
                              using pointee_t = std::remove_const_t<T>;
                              const pointee_t *buf = tensor.buffer();
                              size_t bytes = tensor.size() * sizeof(T);
                              switch (inputTensor->type)
                              {
                              case kTfLiteInt8:
                                  if (bytes == inputTensor->bytes)
                                  {
                                      memcpy(inputTensor->data.raw, buf, tensor.size() * sizeof(T));
                                      inputReceived |= (1 << dstPort);
                                  }
                                  break;
                              break;
                              default:
                                  CMSISSTREAM_LOG_ERR("TFLite: Unsupported tensor input data type %d\n", inputTensor->type);
                                  return;
                              }
                            
                         });
        tryInference();
    }

    cg_status processEvent(int dstPort, Event &&evt) final override
    {

        if (evt.event_id == kValue)
        {
            if (evt.wellFormed<TensorPtr<float>>())
            {
                    convertReceivedF32Tensor(dstPort, std::move(evt.get<TensorPtr<float>>()));
            }
            if (evt.wellFormed<TensorPtr<const float>>())
            {
                    convertReceivedF32Tensor(dstPort, std::move(evt.get<TensorPtr<const float>>()));
            }
            if (evt.wellFormed<TensorPtr<float>>())
            {
                    convertReceivedInt8Tensor(dstPort, std::move(evt.get<TensorPtr<float>>()));
            }
            if (evt.wellFormed<TensorPtr<const float>>())
            {
                    convertReceivedInt8Tensor(dstPort, std::move(evt.get<TensorPtr<const float>>()));
            }
            
           
        }
        return CG_SUCCESS;
    }

    void subscribe(int outputPort, StreamNode &dst, int dstPort) final override
    {
        if ((uint32_t)outputPort >= (1+this->GetNumOutputs()))
            return;
        if (outputPort < 0)
            return;
        if (ev.empty())
            return;
        if (ev.size() <= (size_t)outputPort)
            return;
        ev[outputPort].subscribe(dst, dstPort);
    }

  protected:
    virtual const tflite::MicroOpResolver &GetOpResolver() = 0;
    virtual bool enlistOperations() = 0;
    virtual size_t GetNumInputs() const = 0;
    virtual size_t GetNumOutputs() const = 0;

    uint8_t *tensorArenaAddr_{nullptr};
    uint32_t tensorArenaSize_{0};

    const tflite::Model *m_pModel{nullptr};                            /* Tflite model pointer. */
    std::unique_ptr<tflite::MicroInterpreter> m_pInterpreter{nullptr}; /* Tflite interpreter. */
    tflite::MicroAllocator *m_pAllocator{nullptr};                     /* Tflite micro allocator. */
    bool m_inited{false};                                              /* Indicates whether this object has been initialised. */
    const uint8_t *m_modelAddr{nullptr};                               /* Model address */
    uint32_t m_modelSize{0};                                           /* Model size */

    std::vector<TfLiteTensor *> m_input{};  /* Model's input tensor pointers. */
    std::vector<TfLiteTensor *> m_output{}; /* Model's output tensor pointers. */
    TfLiteType m_type{kTfLiteNoType};       /* Model's data type. */
    bool initErrorOccured{false};
    uint32_t inputReceived{0};

    std::vector<EventOutput> ev;
};
