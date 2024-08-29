#include "factories/ModelFactory.hpp"
#include "models/ResNet10SSDFaceDetector.hpp"
#include "models/Yolo5sPersonDetector.hpp"
#include "cxxopts.hpp"

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <iostream>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <thread>
#include <memory>
#include <map>


std::unique_ptr<IModelDnnDetector> detector;

static GstFlowReturn on_new_sample(GstAppSink *appsink, gpointer user_data) {
    GstSample *sample = gst_app_sink_pull_sample(appsink);

    if (sample) {
        GstBuffer *buffer_in = gst_sample_get_buffer(sample);
        GstBuffer *buffer_out = gst_buffer_new_allocate(nullptr, 640*480*3, nullptr);
        if (!buffer_in || !buffer_out) {
            std::cerr << "Can't allocate gstreamer buffer" << std::endl;
            goto exit;
        }
        GstMapInfo mapIn, mapOut;


        if (gst_buffer_map(buffer_in, &mapIn, GST_MAP_READ) && gst_buffer_map(buffer_out, &mapOut, GST_MAP_WRITE)) {
            try {
                auto start = std::chrono::high_resolution_clock::now();
                bool result = detector->Detect(mapIn.data, mapOut.data);
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                if (!result) {
                    std::cerr << "Can't detect any objects" << std::endl;
                    goto unmap;
                }
            } catch (std::exception& e) {
                std::cerr << "Detector error: " << e.what() << std::endl;
            }

            GstElement *appsrc = (GstElement *)user_data;
            GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(appsrc), buffer_out);
            if (ret != GST_FLOW_OK) {
                std::cerr << "Error during sending frame to video codec" << std::endl;
            }
        }
unmap:
        gst_buffer_unmap(buffer_in, &mapIn);
        gst_buffer_unmap(buffer_out, &mapOut);
exit:
        gst_sample_unref(sample);
    }

    return GST_FLOW_OK;
}

static void printSupportedModels() {
    auto models = ModelFactory::factory;
    for (const auto& modelUnit : models) {
        std::cout << "  " << modelUnit.first << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::string model_dir;
    std::string model_name;
    float conf_threshold;
    int video_id;
    std::string dst_ip;
    std::string dst_port;
    std::string video_device = "/dev/video";

    try {
        cxxopts::Options options("odetect", "Detection of objects based on DNN");

        options.add_options()
            ("d,model_directory", "Model Directory", cxxopts::value<std::string>()->default_value("/usr/share/odetect"))
            ("name", "Model Name", cxxopts::value<std::string>()->default_value("ResNet10SSDFaceDetector"))
            ("t,threshold", "Model Confidence Threshold (0..1]", cxxopts::value<float>()->default_value("0.6"))
            ("v,video_device", "Video Device ID", cxxopts::value<int>())
            ("dst_ip", "Destination IP", cxxopts::value<std::string>())
            ("dst_port", "Destination Port", cxxopts::value<std::string>()->default_value("5000"))
            ("l", "List models")
            ("h,help", "Print usage");

        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << "Usage:\n  odetect -v <video_device> --dst_ip <destination_ip> [OPTION...]\n";
            std::cout << "Warning: video output is only RTP/H264 (program encoded)\n\n";
            std::cout << options.help() << std::endl;
            return 0;
        }
    
        if (result.count("l")) {
            printSupportedModels();
            return 0;
        }

        if (!result.count("video_device") || !result.count("dst_ip")) {
            std::cerr << "Error: Video Device ID and Destination IP are mandatory." << std::endl;
            std::cout << options.help() << std::endl;
            return 1;
        }

        model_dir = result["model_directory"].as<std::string>();
        model_name = result["name"].as<std::string>();
        conf_threshold = result["threshold"].as<float>();
        int video_device_id = result["video_device"].as<int>();
        video_device += std::to_string(video_device_id);
        dst_ip = result["dst_ip"].as<std::string>();
        dst_port = result["dst_port"].as<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        return 1;
    }

    std::chrono::milliseconds timeout(500);

    ODCaps inCaps;
    if (GetOdCapsFromVideoDev(video_device.c_str(), &inCaps)) {
        std::cerr << "Can't get caps for device: " << video_device << std::endl;
        return -1;
    }

    try {
        auto model_unit = ModelFactory::factory.find(model_name);
        if (model_unit == ModelFactory::factory.end()) {
            std::cerr << "Incorrect model name. Call odetect -l\n";
            return -1;
        }
        auto constructFunc = model_unit->second;
        detector = constructFunc(model_dir, inCaps, &conf_threshold);
    } catch (std::exception& e) {
        std::cerr << "Can't allocate detector model: " << e.what() << std::endl;
        return -1;
    }

    gst_init(nullptr, nullptr);

    std::string pipeline_capture_str = "v4l2src device=" + video_device + " ! video/x-raw ! appsink name=mysink";
    GstElement *pipeline_capture = gst_parse_launch(pipeline_capture_str.c_str(), NULL);
    std::cout << "Capture pipeline: " << pipeline_capture_str << std::endl;

    std::string pipeline_encode_str = "appsrc name=source caps=video/x-raw,width=" + std::to_string(inCaps.width)
        + ",height=" + std::to_string(inCaps.height) + ",framerate=30/1,format=BGR ! videoconvert ! x264enc tune=zerolatency speed-preset=superfast key-int-max=15 ! h264parse ! rtph264pay config-interval=1 pt=96 ! udpsink host=" + dst_ip + " port=" + dst_port;
    GstElement *pipeline_encode = gst_parse_launch(
        pipeline_encode_str.c_str(),
        NULL
    );

    std::cout << "Encode pipeline: " << pipeline_encode_str << std::endl;

    GstElement *appsrc = gst_bin_get_by_name(GST_BIN(pipeline_encode), "source");
    GstElement *appsink = gst_bin_get_by_name(GST_BIN(pipeline_capture), "mysink");
    
    g_object_set(appsink, "emit-signals", TRUE, "sync", FALSE, NULL);
    g_signal_connect(appsink, "new-sample", G_CALLBACK(on_new_sample), appsrc);

    std::cout << "Detection starting..." << std::endl;
    gst_element_set_state(pipeline_capture, GST_STATE_PLAYING);
    GstStateChangeReturn ret = gst_element_get_state(pipeline_capture, NULL, NULL, GST_CLOCK_TIME_NONE);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "Failed to start capture pipeline." << std::endl;
        return -1;
    }

    auto start_time = std::chrono::steady_clock::now();
    ret = GST_STATE_CHANGE_FAILURE;
    while(ret == GST_STATE_CHANGE_FAILURE) {
        if (std::chrono::steady_clock::now() - start_time > timeout) {
            std::cerr << "Failed to start detection" << std::endl;
            return -1;
        }
        gst_element_set_state(pipeline_encode, GST_STATE_PLAYING);
        ret = gst_element_get_state(pipeline_encode, NULL, NULL, GST_CLOCK_TIME_NONE);

        std::this_thread::sleep_for(std::chrono::milliseconds(timeout.count() / 10));
    }

    std::cout << "Detection started" << std::endl;

    GstBus *bus = gst_element_get_bus(pipeline_capture);
    GstMessage *msg;
    bool terminate = false;

    while (!terminate) {
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, 
                static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS | GST_MESSAGE_STATE_CHANGED));

        if (msg != nullptr) {
            GError *err;
            gchar *debug_info;

            switch (GST_MESSAGE_TYPE(msg)) {
                case GST_MESSAGE_ERROR:
                    gst_message_parse_error(msg, &err, &debug_info);
                    std::cerr << "Error GStreamer: " << err->message << std::endl;
                    g_clear_error(&err);
                    g_free(debug_info);
                    terminate = true;
                    break;
                case GST_MESSAGE_EOS:
                    std::cout << "End of stream" << std::endl;
                    terminate = true;
                    break;
                case GST_MESSAGE_STATE_CHANGED:
                    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline_capture) || GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline_encode)) {
                        GstState old_state, new_state, pending_state;
                        gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
                        std::cout << "State changed: " << gst_element_state_get_name(old_state) << " -> " 
                                  << gst_element_state_get_name(new_state) << std::endl;
                    }
                    break;
                default:
                    break;
            }
            gst_message_unref(msg);
        }
    }

    gst_object_unref(bus);
    gst_element_set_state(pipeline_capture, GST_STATE_NULL);
    gst_element_set_state(pipeline_encode, GST_STATE_NULL);
    gst_object_unref(pipeline_capture);
    gst_object_unref(pipeline_encode);

    return 0;
}