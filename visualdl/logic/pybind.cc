/* Copyright (c) 2017 VisualDL Authors. All Rights Reserve.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include <ctype.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "visualdl/logic/sdk.h"

namespace py = pybind11;
namespace vs = visualdl;
namespace cp = visualdl::components;

#define ADD_FULL_TYPE_IMPL(CODE) \
  CODE(int32_t);                 \
  CODE(int64_t);                 \
  CODE(float);                   \
  CODE(double);

PYBIND11_MODULE(core, m) {
  m.doc() = R"pbdoc(

        VisualDL uses PyBind to operate with the c++ framework. Users should use LogWriter to instantiate scalar/histogram/image writer

        .. autoclass:: ScalarWriter__float
            :members:

        .. autoclass:: HistogramWriter__float
            :members:

        .. autoclass:: ImageWriter
            :members:

        .. autoclass:: TextWriter
            :members:

        .. autoclass:: AudioWriter
            :members:

    )pbdoc";

  py::class_<vs::LogReader>(m, "LogReader")
      .def(py::init(
          [](const std::string& dir) { return new vs::LogReader(dir); }))
      .def("as_mode", &vs::LogReader::AsMode)
      .def("set_mode", &vs::LogReader::SetMode)
      .def("modes", [](vs::LogReader& self) { return self.storage().modes(); })
      .def("tags", &vs::LogReader::tags)

// clang-format off
      #define READER_ADD_SCALAR(T)                                               \
      .def("get_scalar_" #T, [](vs::LogReader& self, const std::string& tag) { \
        auto tablet = self.tablet(tag);                                        \
        return vs::components::ScalarReader<T>(std::move(tablet));             \
      })
      READER_ADD_SCALAR(float)
      READER_ADD_SCALAR(double)
      READER_ADD_SCALAR(int)
      #undef READER_ADD_SCALAR

      #define READER_ADD_HISTOGRAM(T)                                            \
      .def("get_histogram_" #T, [](vs::LogReader& self, const std::string& tag) { \
          auto tablet = self.tablet(tag);                               \
          return vs::components::HistogramReader<T>(std::move(tablet));    \
        })
      READER_ADD_HISTOGRAM(float)
      READER_ADD_HISTOGRAM(double)
      READER_ADD_HISTOGRAM(int)
      #undef READER_ADD_HISTOGRAM

      // clang-format on
      .def("get_image",
           [](vs::LogReader& self, const std::string& tag) {
             auto tablet = self.tablet(tag);
             return vs::components::ImageReader(self.mode(), tablet);
           })
      .def("get_text",
           [](vs::LogReader& self, const std::string& tag) {
             auto tablet = self.tablet(tag);
             return vs::components::TextReader(tablet);
           })
      .def("get_audio",
           [](vs::LogReader& self, const std::string& tag) {
             auto tablet = self.tablet(tag);
             return vs::components::AudioReader(self.mode(), tablet);
           })
      .def("get_embedding", [](vs::LogReader& self, const std::string& tag) {
        auto tablet = self.tablet(tag);
        return vs::components::EmbeddingReader(tablet);
      });

  // clang-format on
  py::class_<vs::LogWriter>(m, "LogWriter")
      .def(py::init([](const std::string& dir, int sync_cycle) {
        return new vs::LogWriter(dir, sync_cycle);
      }))
      .def("set_mode", &vs::LogWriter::SetMode)
      .def("as_mode", &vs::LogWriter::AsMode)
      .def("save", &vs::LogWriter::Save)
// clang-format off
      #define WRITER_ADD_SCALAR(T)                                               \
      .def("new_scalar_" #T, [](vs::LogWriter& self, const std::string& tag) { \
        auto tablet = self.AddTablet(tag);                                     \
        return cp::Scalar<T>(tablet);                                          \
      })
      #define WRITER_ADD_HISTOGRAM(T)                                           \
      .def("new_histogram_" #T,                                               \
          [](vs::LogWriter& self, const std::string& tag, int num_buckets) { \
            auto tablet = self.AddTablet(tag);                               \
            return cp::Histogram<T>(tablet, num_buckets);                    \
          })
      WRITER_ADD_SCALAR(float)
      WRITER_ADD_SCALAR(double)
      WRITER_ADD_SCALAR(int)
      WRITER_ADD_HISTOGRAM(float)
      WRITER_ADD_HISTOGRAM(double)
      WRITER_ADD_HISTOGRAM(int)
      // clang-format on
      .def("new_image",
           [](vs::LogWriter& self,
              const std::string& tag,
              int num_samples,
              int step_cycle) {
             auto tablet = self.AddTablet(tag);
             return vs::components::Image(tablet, num_samples, step_cycle);
           })
      .def("new_text",
           [](vs::LogWriter& self, const std::string& tag) {
             auto tablet = self.AddTablet(tag);
             return vs::components::Text(tablet);
           })
      .def("new_audio",
           [](vs::LogWriter& self,
              const std::string& tag,
              int num_samples,
              int step_cycle) {
             auto tablet = self.AddTablet(tag);
             return vs::components::Audio(tablet, num_samples, step_cycle);
           })
      .def("new_embedding", [](vs::LogWriter& self, const std::string& tag) {
        auto tablet = self.AddTablet(tag);
        return vs::components::Embedding(tablet);
      });

//------------------- components --------------------
#define ADD_SCALAR_READER(T)                               \
  py::class_<cp::ScalarReader<T>>(m, "ScalarReader__" #T)  \
      .def("records", &cp::ScalarReader<T>::records)       \
      .def("timestamps", &cp::ScalarReader<T>::timestamps) \
      .def("ids", &cp::ScalarReader<T>::ids)               \
      .def("caption", &cp::ScalarReader<T>::caption);
  ADD_SCALAR_READER(int);
  ADD_SCALAR_READER(float);
  ADD_SCALAR_READER(double);
  ADD_SCALAR_READER(int64_t);
#undef ADD_SCALAR_READER

#define ADD_SCALAR_WRITER(T)                                                \
  py::class_<cp::Scalar<T>>(                                                \
      m,                                                                    \
      "ScalarWriter__" #T,                                                  \
      R"pbdoc(PyBind class. Must instantiate through the LogWriter.)pbdoc") \
      .def("set_caption", &cp::Scalar<T>::SetCaption)                       \
      .def("add_record",                                                    \
           &cp::Scalar<T>::AddRecord,                                       \
           R"pbdoc(add a record with the step and value)pbdoc");
  ADD_SCALAR_WRITER(int);
  ADD_SCALAR_WRITER(float);
  ADD_SCALAR_WRITER(double);
#undef ADD_SCALAR_WRITER

  // clang-format on
  py::class_<cp::Image>(m, "ImageWriter", R"pbdoc(
        PyBind class. Must instantiate through the LogWriter.
      )pbdoc")
      .def("set_caption", &cp::Image::SetCaption, R"pbdoc(
        PyBind class. Must instantiate through the LogWriter.
      )pbdoc")
      .def("start_sampling", &cp::Image::StartSampling, R"pbdoc(
        Start a sampling period, this interface will start a new reservoir sampling phase.
      )pbdoc")
      .def("is_sample_taken", &cp::Image::IndexOfSampleTaken, R"pbdoc(
        Will this sample be taken, this interface is introduced to reduce the cost
        of copy image data, by testing whether this image will be sampled, and only
        copy data when it should be sampled. In that way, most of un-sampled image
        data need not be copied or processed at all.

        :return: Index
        :rtype: integer
              )pbdoc")
      .def("finish_sampling", &cp::Image::FinishSampling, R"pbdoc(
        End a sampling period, it will clear all states for reservoir sampling.
      )pbdoc")
      .def("set_sample", &cp::Image::SetSample, R"pbdoc(
        Store the flatten image data as vector of float types. Image params need to be
        specified as a tuple of 3 integers for [width, height, number of channels(3 for RGB)]

        :param index:
        :type index: integer
        :param image_shape: [width, height, number of channels(3 for RGB)]
        :type image_shape: tuple
        :param image_data: Flatten image data
        :type image_data: list
              )pbdoc")
      .def("add_sample", &cp::Image::AddSample, R"pbdoc(
        A combined interface for is_sample_taken and set_sample, simpler but is less efficient.
        Image shape params details see set_sample

        :param image_shape: [width, height, number of channels(3 for RGB)]
        :type image_shape: tuple
        :param image_data: Flatten image data
        :type image_data: list
              )pbdoc");

  py::class_<cp::ImageReader::ImageRecord>(m, "ImageRecord")
      // TODO(ChunweiYan) make these copyless.
      .def("data", [](cp::ImageReader::ImageRecord& self) { return self.data; })
      .def("shape",
           [](cp::ImageReader::ImageRecord& self) { return self.shape; })
      .def("step_id",
           [](cp::ImageReader::ImageRecord& self) { return self.step_id; });

  py::class_<cp::ImageReader>(m, "ImageReader")
      .def("caption", &cp::ImageReader::caption)
      .def("num_records", &cp::ImageReader::num_records)
      .def("num_samples", &cp::ImageReader::num_samples)
      .def("record", &cp::ImageReader::record)
      .def("timestamp", &cp::ImageReader::timestamp);

  py::class_<cp::Text>(m, "TextWriter", R"pbdoc(
        PyBind class. Must instantiate through the LogWriter.
      )pbdoc")
      .def("set_caption", &cp::Text::SetCaption)
      .def("add_record", &cp::Text::AddRecord, R"pbdoc(
            Add a record with the step and text value.

            :param step: Current step value
            :type step: integer
            :param text: Text record
            :type text: basestring
          )pbdoc");

  py::class_<cp::TextReader>(m, "TextReader")
      .def("records", &cp::TextReader::records)
      .def("ids", &cp::TextReader::ids)
      .def("timestamps", &cp::TextReader::timestamps)
      .def("caption", &cp::TextReader::caption)
      .def("total_records", &cp::TextReader::total_records)
      .def("size", &cp::TextReader::size);

  py::class_<cp::Embedding>(m, "EmbeddingWriter", R"pbdoc(
        PyBind class. Must instantiate through the LogWriter.
      )pbdoc")
      .def("set_caption", &cp::Embedding::SetCaption)
      .def("add_embeddings_with_word_list",
           &cp::Embedding::AddEmbeddingsWithWordList)
      .def("add_embeddings_with_word_dict",
           &cp::Embedding::AddEmbeddingsWithWordDict,
           R"pbdoc(
            Add the embedding record. Each run can only store one embedding data. **embeddings** and **word_dict** should be
            the same length. The **word_dict** is used to find the word embedding index in **embeddings**::

                embeddings = [[-1.5246837, -0.7505612, -0.65406495, -1.610278],
                 [-0.781105, -0.24952792, -0.22178008, 1.6906816]]

                word_dict = {"Apple" : 0, "Orange": 1}

            Shows that ``"Apple"`` is embedded to ``[-1.5246837, -0.7505612, -0.65406495, -1.610278]`` and
            ``"Orange"`` is embedded to ``[-0.781105, -0.24952792, -0.22178008, 1.6906816]``

            :param embeddings: list of word embeddings
            :type embeddings: list
            :param word_dict: The mapping from words to indices.
            :type word_dict: dictionary
            )pbdoc");

  py::class_<cp::EmbeddingReader>(m, "EmbeddingReader")
      .def("get_all_labels", &cp::EmbeddingReader::get_all_labels)
      .def("get_all_embeddings", &cp::EmbeddingReader::get_all_embeddings)
      .def("ids", &cp::EmbeddingReader::ids)
      .def("timestamps", &cp::EmbeddingReader::timestamps)
      .def("caption", &cp::EmbeddingReader::caption)
      .def("total_records", &cp::EmbeddingReader::total_records)
      .def("size", &cp::EmbeddingReader::size);

  py::class_<cp::Audio>(m, "AudioWriter", R"pbdoc(
            PyBind class. Must instantiate through the LogWriter.
          )pbdoc")
      .def("set_caption", &cp::Audio::SetCaption, R"pbdoc(
            PyBind class. Must instantiate through the LogWriter.
          )pbdoc")
      .def("start_sampling", &cp::Audio::StartSampling, R"pbdoc(
            Start a sampling period, this interface will start a new reservoir sampling phase.
          )pbdoc")
      .def("is_sample_taken", &cp::Audio::IndexOfSampleTaken, R"pbdoc(
            Will this sample be taken, this interface is introduced to reduce the cost
            of copy audio data, by testing whether this audio will be sampled, and only
            copy data when it should be sampled. In that way, most of un-sampled audio
            data need not be copied or processed at all.

            :return: Index
            :rtype: integer
                  )pbdoc")
      .def("finish_sampling", &cp::Audio::FinishSampling, R"pbdoc(
            End a sampling period, it will clear all states for reservoir sampling.
          )pbdoc")
      .def("set_sample", &cp::Audio::SetSample, R"pbdoc(
            Store the flatten audio data as vector of uint8 types. Audio params need to
            be specified as a tuple of 3 integers as following:
            sample_rate: number of samples(frames) per second, e.g. 8000, 16000 or 44100
            sample_width: size of each sample(frame) in bytes, 16bit frame will be 2
            num_channels: number of channels associated with the audio data, normally 1 or 2

            :param index:
            :type index: integer
            :param audio_params: [sample rate, sample width, number of channels]
            :type audio_params: tuple
            :param audio_data: Flatten audio data
            :type audio_data: list
                  )pbdoc")
      .def("add_sample", &cp::Audio::AddSample, R"pbdoc(
            A combined interface for is_sample_taken and set_sample, simpler but is less efficient.
            Audio params details see set_sample

            :param audio_params: [sample rate, sample width, number of channels]
            :type audio_params: tuple
            :param audio_data: Flatten audio data
            :type audio_data: list of uint8
                  )pbdoc");

  py::class_<cp::AudioReader::AudioRecord>(m, "AudioRecord")
      // TODO(Nicky) make these copyless.
      .def("data", [](cp::AudioReader::AudioRecord& self) { return self.data; })
      .def("shape",
           [](cp::AudioReader::AudioRecord& self) { return self.shape; })
      .def("step_id",
           [](cp::AudioReader::AudioRecord& self) { return self.step_id; });

  py::class_<cp::AudioReader>(m, "AudioReader")
      .def("caption", &cp::AudioReader::caption)
      .def("num_records", &cp::AudioReader::num_records)
      .def("num_samples", &cp::AudioReader::num_samples)
      .def("record", &cp::AudioReader::record)
      .def("timestamp", &cp::AudioReader::timestamp);

#define ADD_HISTOGRAM_WRITER(T)                                             \
  py::class_<cp::Histogram<T>>(                                             \
      m,                                                                    \
      "HistogramWriter__" #T,                                               \
      R"pbdoc(PyBind class. Must instantiate through the LogWriter.)pbdoc") \
      .def("add_record",                                                    \
           &cp::Histogram<T>::AddRecord,                                    \
           R"pbdoc(add a record with the step and histogram_value)pbdoc");
  ADD_FULL_TYPE_IMPL(ADD_HISTOGRAM_WRITER)
#undef ADD_HISTOGRAM_WRITER

#define ADD_HISTOGRAM_RECORD_INSTANCE(T)                                      \
  py::class_<vs::HistogramRecord<T>::Instance>(m, "HistogramInstance__" #T)   \
      .def("left",                                                            \
           [](typename vs::HistogramRecord<T>::Instance& self) {              \
             return self.left;                                                \
           })                                                                 \
      .def("right",                                                           \
           [](typename vs::HistogramRecord<T>::Instance& self) {              \
             return self.right;                                               \
           })                                                                 \
      .def("frequency", [](typename vs::HistogramRecord<T>::Instance& self) { \
        return self.frequency;                                                \
      });

  ADD_FULL_TYPE_IMPL(ADD_HISTOGRAM_RECORD_INSTANCE)
#undef ADD_HISTOGRAM_RECORD_INSTANCE

#define ADD_HISTOGRAM_RECORD(T)                                            \
  py::class_<vs::HistogramRecord<T>>(m, "HistogramRecord__" #T)            \
      .def("step", [](vs::HistogramRecord<T>& self) { return self.step; }) \
      .def("timestamp",                                                    \
           [](vs::HistogramRecord<T>& self) { return self.timestamp; })    \
      .def("instance", &vs::HistogramRecord<T>::instance)                  \
      .def("num_instances", &vs::HistogramRecord<T>::num_instances);
  ADD_FULL_TYPE_IMPL(ADD_HISTOGRAM_RECORD)
#undef ADD_HISTOGRAM_RECORD

#define ADD_HISTOGRAM_READER(T)                                 \
  py::class_<cp::HistogramReader<T>>(m, "HistogramReader__" #T) \
      .def("num_records", &cp::HistogramReader<T>::num_records) \
      .def("record", &cp::HistogramReader<T>::record);
  ADD_FULL_TYPE_IMPL(ADD_HISTOGRAM_READER)
#undef ADD_HISTOGRAM_READER

}  // end pybind
