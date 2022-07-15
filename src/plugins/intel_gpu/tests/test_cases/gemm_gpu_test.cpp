// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "test_utils.h"

#include <intel_gpu/primitives/input_layout.hpp>
#include <intel_gpu/primitives/gemm.hpp>
#include <intel_gpu/primitives/crop.hpp>

#include <cstddef>

using namespace cldnn;
using namespace ::tests;

TEST(gemm_gpu, basic_bfyx_t1) {
    auto& engine = get_test_engine();
    auto input = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 3, 4 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 1, 4 } });

    std::vector<float> input_data = {
        1.f, -2.f,  3.f,
        -4.f, 5.f,  6.f,
        1.f, 2.f, 3.f,
        3.f, 2.f, -1.f,

    };

    std::vector<float> input_data2 = {
        2.f,
        5.f,
        -4.f,
        -7.f,
    };
    set_values(input, input_data);
    set_values(input2, input_data2);

    std::vector<float> out_data = {
        -43.f, -1.f, 31.f
    };

    topology topology;
    topology.add(
        input_layout("input", input->get_layout())
    );
    topology.add(
        input_layout("input2", input2->get_layout())
    );
    topology.add(
        gemm("output", { "input", "input2" }, data_types::f32, true, false)
    );

    network network(engine, topology);
    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    auto outputs = network.execute();

    auto output = outputs.at("output").get_memory();
    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    EXPECT_EQ(output_ptr.size(), (uint32_t)3);
    for (uint32_t i = 0; i < out_data.size(); ++i) {
        EXPECT_FLOAT_EQ(output_ptr[i], out_data[i]);
    }
}
TEST(gemm_gpu, basic_bfyx_t2) {
    auto& engine = get_test_engine();
    auto input = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 4, 3 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 4, 1 } });

    std::vector<float> input_data = {
        1.f, -2.f,  3.f, -4.f,
        5.f,  6.f, 1.f, 2.f,
        3.f, 3.f, 2.f, -1.f,

    };

    std::vector<float> input_data2 = {
        2.f, 5.f, -4.f, -7.f,
    };
    set_values(input, input_data);
    set_values(input2, input_data2);

    std::vector<float> out_data = {
        8.f, 22.f, 20.f
    };

    topology topology;
    topology.add(
        input_layout("input", input->get_layout())
    );
    topology.add(
        input_layout("input2", input2->get_layout())
    );
    topology.add(
        gemm("output", { "input", "input2" }, data_types::f32, false, true)
    );

    network network(engine, topology);
    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    auto outputs = network.execute();

    auto output = outputs.at("output").get_memory();
    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    EXPECT_EQ(output_ptr.size(), (uint32_t)3);
    for (uint32_t i = 0; i < out_data.size(); ++i) {
        EXPECT_FLOAT_EQ(output_ptr[i], out_data[i]);
    }
}

TEST(gemm_gpu, basic_bfyx_t2_inplace_crop_with_pad) {
    auto& engine = get_test_engine();
    auto input = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 2, 4, 3 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 4, 1 } });

    std::vector<float> input_data = {
        1.f, -2.f,  3.f, -4.f,
        5.f,  6.f, 1.f, 2.f,
        3.f, 3.f, 2.f, -1.f,

        1.f, -2.f,  3.f, -4.f,
        5.f,  6.f, 1.f, 2.f,
        3.f, 3.f, 2.f, -1.f,
    };

    std::vector<float> input_data2 = {
        2.f, 5.f, -4.f, -7.f,
    };
    set_values(input, input_data);
    set_values(input2, input_data2);

    std::vector<float> out_data = {
        8.f, 22.f, 20.f
    };

    topology topology;
    topology.add(
        input_layout("input", input->get_layout())
    );
    topology.add(
        input_layout("input2", input2->get_layout())
    );
    topology.add(
        crop("crop.1", "input", { 1, 1, 4, 3 }, { 0, 1, 0, 0 })
    );
    topology.add(
        gemm("output", { "crop.1", "input2" }, data_types::f32, false, true)
    );

    build_options options;
    options.set_option(build_option::optimize_data(true));
    network network(engine, topology, options);
    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    auto outputs = network.execute();

    auto output = outputs.at("output").get_memory();
    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    EXPECT_EQ(output_ptr.size(), (uint32_t)3);
    for (uint32_t i = 0; i < out_data.size(); ++i) {
        EXPECT_FLOAT_EQ(output_ptr[i], out_data[i]);
    }
}

TEST(gemm_gpu, basic_bfyx_t1t2) {
    auto& engine = get_test_engine();
    auto input = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 1, 3, 4 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 1, 4, 1 } });

    std::vector<float> input_data = {
        1.f, -2.f,  3.f,
        -4.f, 5.f,  6.f,
        1.f, 2.f, 3.f,
        3.f, 2.f, -1.f,

        1.f, -2.f,  3.f,
        -4.f, 5.f,  6.f,
        1.f, 2.f, 3.f,
        3.f, 2.f, -1.f,

    };

    std::vector<float> input_data2 = {
        2.f, 5.f, -4.f, -7.f,

        2.f, 5.f, -4.f, -7.f,
    };
    set_values(input, input_data);
    set_values(input2, input_data2);

    std::vector<float> out_data = {
        -43.f, -1.f, 31.f,
        -43.f, -1.f, 31.f
    };

    topology topology;
    topology.add(
        input_layout("input", input->get_layout())
    );
    topology.add(
        input_layout("input2", input2->get_layout())
    );
    topology.add(
        gemm("output", { "input", "input2" }, data_types::f32, true, true)
    );

    network network(engine, topology);
    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    auto outputs = network.execute();

    auto output = outputs.at("output").get_memory();
    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    EXPECT_EQ(output_ptr.size(), (uint32_t)6);
    for (uint32_t i = 0; i < out_data.size(); ++i) {
        EXPECT_FLOAT_EQ(output_ptr[i], out_data[i]);
    }
}

TEST(gemm_gpu, basic_input3) {
    auto& engine = get_test_engine();
    auto input = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 3, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 2, 3 } });
    auto input3 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 2, 2 } });
    float alpha = 2.f;
    float beta = 10.f;

    std::vector<float> input_data = {
        1.0f, 2.0f, 3.0f,
        1.0f, 0.0f, 1.0f
    };

    std::vector<float> input_data2 = {
        3.0f, 3.0f,
        1.0f, 2.0f,
        1.0f, 2.0f,
    };

    std::vector<float> input3_data = {
        1.0f, 0.0f,
        2.0f, 0.0f,
    };

    set_values(input, input_data);
    set_values(input2, input_data2);
    set_values(input3, input3_data);

    std::vector<float> out_data = {
        26.0f, 26.0f,
        28.0f, 10.0f
    };

    topology topology;
    topology.add(
        input_layout("input", input->get_layout())
    );
    topology.add(
        input_layout("input2", input2->get_layout())
    );
    topology.add(
        input_layout("input3", input3->get_layout())
    );
    topology.add(
        gemm("output", { "input", "input2", "input3" }, data_types::f32,  false, false, alpha, beta)
    );

    network network(engine, topology);
    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("input3", input3);
    auto outputs = network.execute();

    auto output = outputs.at("output").get_memory();
    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    EXPECT_EQ(output_ptr.size(), (uint32_t)4);

    for (uint32_t i = 0; i < out_data.size(); ++i) {
        EXPECT_FLOAT_EQ(output_ptr[i], out_data[i]);
    }
}

TEST(gemm_gpu, basic_input3_t1t2) {
    auto& engine = get_test_engine();
    auto input = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 4, 3 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 3, 2 } });
    auto input3 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 2, 4 } });
    float alpha = 2.f;
    float beta = 3.f;

    std::vector<float> input_data = {
        1.0f, 2.0f, 3.0f, 4.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };

    std::vector<float> input_data2 = {
        3.0f, 3.0f, 1.0f,
        2.0f, 1.0f, 2.0f,
    };

    std::vector<float> input3_data = {
        1.0f, 0.0f,
        1.0f, 0.0f,
        2.0f, 2.0f,
        1.0f, 1.0f,
    };

    set_values(input, input_data);
    set_values(input2, input_data2);
    set_values(input3, input3_data);

    std::vector<float> out_data = {
        15.0f, 6.0f,
        15.0f, 8.0f,
        30.0f, 20.0f,
        27.0f, 19.0f
    };

    topology topology;
    topology.add(
        input_layout("input", input->get_layout())
    );
    topology.add(
        input_layout("input2", input2->get_layout())
    );
    topology.add(
        input_layout("input3", input3->get_layout())
    );
    topology.add(
        gemm("output", { "input", "input2", "input3" }, data_types::f32, true, true, alpha, beta)
    );

    network network(engine, topology);
    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("input3", input3);
    auto outputs = network.execute();

    auto output = outputs.at("output").get_memory();
    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    EXPECT_EQ(output_ptr.size(), (uint32_t)8);

    for (uint32_t i = 0; i < out_data.size(); ++i) {
        EXPECT_FLOAT_EQ(output_ptr[i], out_data[i]);
    }
}
TEST(gemm_gpu, basic_input3_1) {
    auto& engine = get_test_engine();
    auto input = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 3, 4 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 2, 3 } });
    auto input3 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 2, 4 } });
    float alpha = 2.f;
    float beta = 3.f;

    std::vector<float> input_data = {
        1.0f, 1.0f, 0.0f,
        2.0f, 0.0f, 0.0f,
        3.0f, 1.0f, 0.0f,
        4.0f, 0.0f, 0.0f
    };

    std::vector<float> input_data2 = {
        3.0f, 2.0f,
        3.0f, 1.0f,
        1.0f, 2.0f,
    };

    std::vector<float> input3_data = {
        1.0f, 0.0f,
        1.0f, 0.0f,
        2.0f, 2.0f,
        1.0f, 1.0f,
    };

    set_values(input, input_data);
    set_values(input2, input_data2);
    set_values(input3, input3_data);

    std::vector<float> out_data = {
        15.0f, 6.0f,
        15.0f, 8.0f,
        30.0f, 20.0f,
        27.0f, 19.0f
    };

    topology topology;
    topology.add(
        input_layout("input", input->get_layout())
    );
    topology.add(
        input_layout("input2", input2->get_layout())
    );
    topology.add(
        input_layout("input3", input3->get_layout())
    );
    topology.add(
        gemm("output", { "input", "input2", "input3" }, data_types::f32, false, false, alpha, beta)

    );

    network network(engine, topology);
    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("input3", input3);
    auto outputs = network.execute();

    auto output = outputs.at("output").get_memory();
    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    EXPECT_EQ(output_ptr.size(), (uint32_t)8);

    for (uint32_t i = 0; i < out_data.size(); ++i) {
        EXPECT_FLOAT_EQ(output_ptr[i], out_data[i]);
    }
}

TEST(gemm_gpu, basic_input3_t2) {
    auto& engine = get_test_engine();
    auto input = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 3, 4 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 3, 2 } });
    auto input3 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 2, 4 } });
    float alpha = 2.f;
    float beta = 3.f;

    std::vector<float> input_data = {
        1.0f, 1.0f, 0.0f,
        2.0f, 0.0f, 0.0f,
        3.0f, 1.0f, 0.0f,
        4.0f, 0.0f, 0.0f
    };

    std::vector<float> input_data2 = {
        3.0f, 3.0f, 1.0f,
        2.0f, 1.0f, 2.0f,
    };

    std::vector<float> input3_data = {
        1.0f, 0.0f,
        1.0f, 0.0f,
        2.0f, 2.0f,
        1.0f, 1.0f,
    };

    set_values(input, input_data);
    set_values(input2, input_data2);
    set_values(input3, input3_data);

    std::vector<float> out_data = {
        15.0f, 6.0f,
        15.0f, 8.0f,
        30.0f, 20.0f,
        27.0f, 19.0f,
    };

    topology topology;
    topology.add(
        input_layout("input", input->get_layout())
    );
    topology.add(
        input_layout("input2", input2->get_layout())
    );
    topology.add(
        input_layout("input3", input3->get_layout())
    );
    topology.add(
        gemm("output", { "input", "input2", "input3" }, data_types::f32, false, true, alpha, beta)
    );

    network network(engine, topology);
    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("input3", input3);
    auto outputs = network.execute();

    auto output = outputs.at("output").get_memory();
    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    EXPECT_EQ(output_ptr.size(), (uint32_t)8);

    for (uint32_t i = 0; i < out_data.size(); ++i) {
        EXPECT_FLOAT_EQ(output_ptr[i], out_data[i]);
    }
}

TEST(gemm_gpu, basic_input3_t1) {
    auto& engine = get_test_engine();
    auto input = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 4, 3 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 2, 3 } });
    auto input3 = engine.allocate_memory({ data_types::f32, format::bfyx, { 1, 1, 2, 4 } });
    float alpha = 2.f;
    float beta = 3.f;

    std::vector<float> input_data = {
        1.0f, 2.0f, 3.0f, 4.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };

    std::vector<float> input_data2 = {
        3.0f, 2.0f,
        3.0f, 1.0f,
        1.0f, 2.0f
    };

    std::vector<float> input3_data = {
        1.0f, 0.0f,
        1.0f, 0.0f,
        2.0f, 2.0f,
        1.0f, 1.0f,
    };

    set_values(input, input_data);
    set_values(input2, input_data2);
    set_values(input3, input3_data);

    std::vector<float> out_data = {
        15.0f, 6.0f,
        15.0f, 8.0f,
        30.0f, 20.0f,
        27.0f, 19.0f,
    };

    topology topology;
    topology.add(
        input_layout("input", input->get_layout())
    );
    topology.add(
        input_layout("input2", input2->get_layout())
    );
    topology.add(
        input_layout("input3", input3->get_layout())
    );
    topology.add(
        gemm("output", { "input", "input2", "input3" }, data_types::f32, true, false, alpha, beta)
    );

    network network(engine, topology);
    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    network.set_input_data("input3", input3);
    auto outputs = network.execute();

    auto output = outputs.at("output").get_memory();
    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    EXPECT_EQ(output_ptr.size(), (uint32_t)8);

    for (uint32_t i = 0; i < out_data.size(); ++i) {
        EXPECT_FLOAT_EQ(output_ptr[i], out_data[i]);
    }
}

TEST(gemm_gpu, basic_bfyx) {
    auto& engine = get_test_engine();
    auto input = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 1, 4, 3 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 1, 1, 4 } });

    std::vector<float> input_data = {
        1.f, -2.f,  3.f,  -4.f,
        5.f,  6.f,  7.f,   8.f,
        -10.f, 12.f, 13.f, -13.f,

        1.f, -2.f,  3.f,  -4.f,
        5.f,  6.f,  7.f,   8.f,
        -10.f, 12.f, 13.f, -13.f,
    };

    std::vector<float> input_data2 = {
        2.f,
        5.f,
        -4.f,
        -7.f,
        2.f,
        5.f,
        -4.f,
        -7.f,
    };
    set_values(input, input_data);
    set_values(input2, input_data2);

    std::vector<float> out_data = {
        8.f, -44.f, 79.f, 8.f, -44.f, 79.f,
    };

    topology topology;
    topology.add(
        input_layout("input", input->get_layout())
    );
    topology.add(
        input_layout("input2", input2->get_layout())
    );
    topology.add(
        gemm("output", { "input", "input2" }, data_types::f32)
    );

    network network(engine, topology);
    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    auto outputs = network.execute();

    auto output = outputs.at("output").get_memory();
    cldnn::mem_lock<float> output_ptr(output, get_test_stream());


    EXPECT_EQ(output_ptr.size(), (uint32_t)6);
    for (uint32_t i = 0; i < out_data.size(); ++i) {
            EXPECT_FLOAT_EQ(output_ptr[i], out_data[i]);
    }
}

TEST(gemm_gpu, basic3_bfyx) {
    auto& engine = get_test_engine();
    auto input = engine.allocate_memory({ data_types::f32, format::bfyx, { 5, 1, 500, 9 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 5, 1, 1, 500 } });

    std::vector<float> input_data = {
        -0.000449777f, -0.000137405f, -0.0762075f, 0.000949166f, 0.0346124f, -0.0111424f, 0.0108357f, 0.0121679f, 0.0242749f, 0.052692f, -0.0017713f, 0.0053728f, 0.0128862f, -0.0162366f, 0.0125041f, -0.00602398f, 0.0107778f, -0.00323086f, -0.00914208f, -0.013884f, 0.00755173f, -0.0175622f, 0.00473339f, -0.015003f, -0.0238219f, 0.004502f, 0.00187154f, 0.0041163f, -9.36184e-05f, 0.00873372f, 0.0121869f, -0.020973f, -0.006006f, -0.0038208f, 0.00210471f, 0.00255549f, -0.0251856f, -0.0626372f, -0.0059258f, -0.0058662f, -0.0946306f, 0.00197436f, 0.00105865f, -0.0033595f, 0.0158977f, -0.0036025f, -0.00568902f, -0.0202577f, -0.000251319f, -0.0117895f, -0.0144239f, -0.0144024f, -0.0150431f, -0.0354826f, -0.0135123f, -0.000422157f, 0.0286438f, -0.000884989f, -0.00675718f, 0.013241f, -0.0118388f, 0.0321394f, -0.000803071f, 0.11408f, -0.00806301f, -0.00831608f, 0.0165189f, 0.016094f, -0.000449332f, -0.00695901f, 0.0437514f, -0.00172117f, 0.00180391f, -0.000859933f, -0.0144826f, 0.0262613f, -0.00194352f, -1.98829e-05f, -0.00902827f, -0.00400867f, -0.00600827f, 0.0120846f, -0.0162493f, 0.0418596f, 0.00131911f, -0.00631566f, 0.00270484f, -0.0950513f, 0.00726431f, -0.0169798f, -0.000554365f, -0.00256903f, -0.00885843f, 0.0104025f, 0.00590779f, -0.00175832f, 0.0168603f, 0.00964353f, -0.0180614f, 0.0213157f, 0.0209548f, -0.0231143f, -0.00121617f, -0.0129815f, -0.0199287f, 0.00863336f, -0.00464991f, 0.0162288f, -0.340115f, -0.011018f, -0.0593997f, 0.00644821f, 0.0416332f, 0.0394596f, 0.0172296f, 0.00494231f, 0.0143805f, -0.00819845f, 0.00196982f, 0.00393258f, 0.0246168f, -0.0235927f, 0.0131416f, -0.0190432f, -0.0237865f, -0.0155627f, 0.0265165f, 0.0162884f, 0.00321098f, 0.0136674f, -0.000966112f, -0.0100813f, -0.00604589f, 0.00889466f, 0.0113945f, 0.0264707f, 0.00371883f, -0.00843358f, 0.0145675f, 0.0048638f, 0.00110399f, -0.00130233f, 0.00740726f, -0.00393368f, -0.0242178f, 0.00341681f, 0.00115369f, -0.00297881f, -0.0844071f, 0.0537151f, -0.00209399f, 0.0310295f, 0.0383914f, 0.00456459f, 0.0188114f, -0.0177144f, 0.0133258f, 0.0584683f, -0.00640495f, 0.0175946f, 0.0186782f, 0.00213311f, 0.00393403f, 0.00382759f, 0.00267507f, 0.00493673f, -0.00856695f, -0.00627955f, -0.0103436f, -0.000671664f, -0.110419f, 0.0307264f, 0.0042176f, 0.0031638f, 0.0154172f, 0.00265482f, 0.0410853f, 0.00833895f, -0.0183989f, -0.000717906f, -0.0090387f, -0.00404523f, -0.00976238f, -0.0137555f, 0.000157289f, -0.00341186f, -0.0214878f, 0.0142639f, 0.00624623f, 0.000537292f, -0.0520912f, -0.0432221f, -0.00330415f, 0.0263942f, -0.00150974f, 0.00172088f, -0.0815726f, -0.0201155f, -0.00986346f, 0.0121252f, 0.00198959f, -0.0349936f, -0.00608366f, -0.00399543f, 0.0192487f, -0.0123156f, 0.0072797f, 0.000507143f, 0.0334805f, 0.000609379f, 0.00961966f, -0.00697663f, 0.00201967f, -0.0207349f, -0.0103385f, -0.00343849f, -0.00330492f, 0.035106f, -0.00456996f, 0.00197528f, 0.016148f, 0.0142903f, 0.0616483f, 0.0093118f, -0.0596028f, 0.00945764f, -0.00659242f, 0.118389f, -0.00259384f, -0.00285344f, 0.00567036f, 0.0195813f, -0.00461807f, -0.0608699f, 0.00380259f, 0.00143385f, -0.00466997f, 0.0194046f, -0.0198423f, -0.00334569f, -0.014399f, 0.0130021f, -0.0141619f, -0.00859914f, 0.00997122f, -0.0198446f, -0.0094162f, -0.0116609f, -0.0111888f, -0.00903524f, 0.00937981f, 0.01772f, -0.00236374f, -0.00870162f, 0.000141193f, -0.0343695f, -0.00997931f, 0.0073531f, -0.100394f, -0.00367661f, -0.00124499f, 0.00318026f, 0.0554203f, -0.00342582f, -0.0104147f, -0.0577869f, -0.0126485f, -0.0332496f, 0.0346141f, 0.0307962f, -0.0174745f, -0.0387339f, 0.0167707f, -0.0363424f, 0.0154902f, -0.0118644f, -4.63543e-06f, -0.0683506f, -0.0344076f, -0.00104884f, -0.00883997f, -0.00305185f, -0.0150299f, -0.0186403f, 0.0110238f, 0.00779224f, -0.0102231f, 0.0087488f, -0.0138988f, -0.0229105f, -0.0244903f, -0.0202919f, 0.00135903f, -0.00574432f, 0.00254918f, 0.0340209f, -0.046428f, -0.00670622f, 0.000925543f, -0.0249251f, -0.00275456f, 0.0199177f, 0.000210993f, 0.027762f, -0.0228046f, 0.0484813f, 0.00538959f, 0.0136714f, -0.00690097f, -0.0448533f, -0.00815204f, 0.00734891f, 0.0173959f, -0.0379109f, 0.0594617f, -0.00722084f, 0.0415935f, 0.014792f, -0.0170252f, -0.0139396f, 0.00146415f, 0.00117702f, 0.0685559f, 0.00727832f, -0.107566f, -0.0112505f, 0.033853f, 0.0046957f, -0.0242369f, 0.0148181f, -0.0723487f, -0.00961667f, 0.0304085f, -0.00520772f, -0.0316467f, 0.0327801f, -0.00755137f, 0.0166041f, -0.0557288f, -0.0227759f, -0.00314548f, 0.0152585f, 0.020071f, -0.0377076f, 0.00687613f, -0.0273935f, -0.00647955f, 0.0105047f, -0.0137238f, 0.023264f, -0.0455722f, -0.00221414f, -0.0258535f, -0.0236395f, 0.0593407f, 0.00448763f, 0.0150777f, 0.00437925f, 0.0295782f, -0.0344752f, 0.00365267f, 0.140464f, -0.0479012f, 0.025726f, 0.119063f, 0.000301925f, -0.00810565f, -0.354073f, -0.0723185f, -0.0046123f, 0.033882f, -0.044552f, -0.0138361f, 0.00384129f, 0.0139111f, -0.01667f, -0.0821503f, 0.0029974f, -0.0306725f, 0.0160366f, 0.0334754f, 0.0192693f, -0.00616713f, -0.00232275f, 0.0107987f, 0.00437057f, 0.0017298f, 0.0196916f, -0.0417255f, -0.00911193f, 0.00876709f, -0.00172422f, -0.00105248f, -0.0191631f, -0.00387423f, -0.0102766f, -0.025317f, -0.0416204f, -0.0319611f, -0.00359193f, 0.00424064f, -0.00575092f, -0.0282402f, 0.0745899f, -0.0126492f, -0.0162564f, -0.261967f, -0.705265f, -0.0403731f, -0.00209634f, -0.694297f, 0.00956909f, 0.0158826f, 0.0130207f, 0.003825f, -0.000300812f, -0.0121346f, 0.00642053f, -0.012902f, 0.0309272f, 0.0609192f, -0.00654145f, -0.0937578f, -0.00432024f, -0.00767539f, 0.0461248f, 0.00701077f, -0.0174477f, 0.00563833f, -0.0107107f, -0.0255275f, 0.00892488f, -0.00166062f, 0.039829f, -0.00150394f, 0.00742194f, -0.00885529f, -0.0103532f, 0.0777858f, 0.0885367f, -0.00425715f, 0.0423651f, -0.0446651f, -0.635069f, -0.00919329f, -0.00356176f, 0.00988705f, 0.0116529f, -0.0401253f, 0.00260105f, 0.00573955f, -0.0667439f, 0.101175f, 0.0765288f, -0.0120077f, 0.00322599f, -0.0192768f, 0.0382749f, -0.222119f, -0.0452036f, 0.0424303f, 0.0890699f, 0.0117557f, 0.0315167f, 0.0284256f, 0.00541845f, -0.250147f, 0.00420668f, -0.0189724f, -0.00416381f, -0.00162803f, -0.0108763f, -0.00970892f, 0.0134476f, -0.0254931f, 0.0307225f, 0.00128596f, 0.0171106f, 0.00467854f, -0.0124376f, 0.0183396f, 0.0021754f, 0.00170886f, -0.0226898f, 0.0250111f, -0.0533301f, -0.0163268f, 0.00618995f, 0.0416378f, 0.0475397f, 0.0105684f, -0.00440933f, 0.0496722f, -0.0215733f, -0.0256361f, -0.0285091f, -0.0276881f, -0.00102202f, -0.0720219f, -0.0296656f,
        0.00465617f, 0.00138814f, -0.0913312f, -0.0161213f, 0.0160887f, 0.0204469f, -0.0223319f, 0.015304f, 0.000397867f, 0.00824013f, 0.0114613f, 0.00408309f, 0.0384456f, -0.00453968f, 0.0176576f, 0.100434f, -0.0393971f, 0.0160015f, -0.00313166f, -0.0058054f, 0.0342083f, 0.0333727f, 0.00275399f, -0.0111393f, -0.0656798f, 0.0117794f, 0.00399766f, 0.00310487f, 0.00290905f, 0.00311256f, 0.0103328f, 0.00221549f, -0.00340486f, -0.00955604f, -0.010614f, 0.0144013f, -0.0244803f, 0.246714f, 0.00585756f, -0.0183366f, 0.0131221f, -0.015529f, 0.0634503f, -0.00107566f, 0.0230663f, -0.00523926f, -0.0100486f, -0.0270644f, 0.0938544f, -0.0136558f, 0.0164469f, -0.349288f, 0.0108305f, 0.0621752f, -0.00813808f, -0.0218271f, 0.0168811f, -0.00509217f, -0.0249135f, 0.0268669f, -0.0294336f, 0.0396944f, -0.00419361f, 0.00843219f, -0.000475472f, -0.0122415f, 0.0142385f, 0.0240099f, -0.0041296f, 0.0167314f, -0.0210217f, -0.00275032f, 0.0121842f, -0.00556776f, -0.0215306f, 0.0411878f, -0.00102203f, 0.00011487f, -0.0142263f, -0.00257424f, -0.0044306f, 0.0115836f, -0.0331884f, 0.0153153f, 0.0023461f, -0.0229996f, -0.00982945f, 0.0207273f, 0.0039542f, -0.0275622f, -0.00118208f, -0.00703868f, -0.0111554f, 0.0155981f, -0.0197133f, -0.00157645f, 0.0790344f, 0.0277319f, -0.0239723f, 0.0133704f, 0.0153687f, -0.0220235f, -0.0652554f, 0.0340702f, -0.0256995f, 0.00463251f, -0.134567f, 0.0048301f, -0.0935251f, -0.0125128f, -0.0560035f, -0.000903825f, 0.0231884f, 0.0678238f, 0.0172834f, 0.0226948f, -0.00784814f, -0.000168366f, 0.0165854f, 0.00979108f, -0.010978f, -0.147669f, 0.020833f, -0.0320907f, -0.339001f, -0.0307849f, -0.00796792f, 0.00704321f, -0.0258511f, 0.0302859f, -0.0174755f, -0.0208662f, -0.00800382f, -0.00772683f, 0.00787931f, 0.0244046f, 0.0635711f, -0.0490687f, 0.00843431f, -0.00969577f, -0.00403176f, -0.00225678f, -0.00425568f, 0.00423476f, -0.0522863f, 0.00901175f, 0.00701737f, 0.0203201f, 0.00764967f, -0.0128627f, -0.0154611f, -0.00973917f, 0.0172989f, 0.00679487f, -0.00897315f, -0.00337138f, -0.0103584f, -0.00507785f, -0.00390477f, 0.0472275f, 0.0060846f, 0.0151745f, 0.0472687f, 0.000490868f, 0.0196255f, 0.00541134f, -0.0206129f, -0.00112977f, -0.0197924f, -0.0553976f, -0.098063f, 0.0664134f, 0.00349375f, 0.00311233f, 0.0401445f, 0.0128354f, -0.0250036f, 0.0436594f, -0.0462325f, -0.00102946f, -0.013474f, -0.0172785f, 0.0394013f, -0.00569089f, 0.000160535f, 0.000504291f, 0.0504433f, -0.0205918f, 0.0101148f, -0.00946464f, -0.0885629f, -0.04032f, -0.012075f, 0.492342f, -0.000999111f, 0.00407901f, 0.0888248f, 0.0100317f, -0.024372f, -0.0211601f, 0.000658811f, -0.0209988f, -0.0190039f, -0.0219266f, -0.0516314f, -0.00642571f, 0.00488745f, 0.00512097f, 0.0145898f, -0.00157307f, 0.0026168f, 0.0156606f, -0.00531944f, -0.017507f, -0.0180003f, 0.00282254f, 0.0143295f, 0.0777137f, -0.00385748f, -0.00549398f, -0.0172826f, 0.0323722f, 0.185825f, 0.0121615f, 0.00399867f, -0.0541097f, 0.0386216f, 0.0595922f, 0.594257f, -0.00955271f, 0.00343269f, 0.0139925f, 0.00328999f, -0.0792421f, -0.045498f, 0.0113837f, -0.00976291f, 0.00624078f, -0.0254107f, -0.0216194f, -0.028773f, 0.0236943f, 0.0197444f, -0.00939094f, 0.0135671f, -0.0407697f, 0.00794318f, -0.0184558f, -0.0282076f, -0.0112124f, 0.00710705f, 0.0203747f, -0.00201855f, -0.0137849f, -0.00224183f, -0.00758043f, 0.0109492f, 0.0111736f, -0.0524165f, -0.00359813f, -0.0105491f, 0.00795013f, 0.0490089f, -0.0172285f, -0.131601f, -0.640844f, -0.00210558f, -0.0191391f, 0.144537f, -0.0187546f, -0.0117677f, -0.0243942f, -0.0673674f, 0.0116665f, -0.00634048f, -0.0171121f, -0.018849f, -0.0452217f, -0.0314511f, 0.01823f, -0.0338747f, -0.00232084f, -0.0184449f, -0.0628265f, -0.00846206f, 0.00285066f, 0.281056f, -0.0109403f, -0.036282f, 0.00725135f, -0.027479f, -0.0120889f, 0.0185699f, -0.00228023f, 0.000971992f, 0.020036f, -0.0437852f, -0.013831f, 0.0284799f, -0.0116033f, -0.0213317f, -0.0391473f, -0.0180216f, 0.0224665f, 0.00661723f, 0.0188164f, -0.00856477f, -0.0188785f, -0.0419517f, -0.0383142f, 0.00822795f, -0.0210551f, 0.0376673f, -0.0158509f, 0.0531296f, -0.0222652f, 0.0202294f, 0.0377989f, -0.0486931f, -0.0236611f, -0.0364076f, -0.0364403f, 0.105507f, -0.0520728f, -0.085646f, -0.0517868f, 0.00898522f, 0.0145328f, -0.0152412f, 0.00230019f, -0.0490983f, 0.0199105f, 0.193699f, -0.00652485f, -0.0293521f, -0.101157f, 0.00759732f, 0.0611226f, 0.00668415f, -0.0644944f, -0.00138395f, -0.0872389f, -0.0289147f, -0.0104552f, 0.0102965f, -0.00918203f, -0.0163947f, 0.00688836f, -0.0460991f, 0.0010579f, -0.0220147f, 0.00389295f, -0.0450669f, -0.0338309f, -0.00643917f, -0.164896f, 0.00520622f, -0.00943891f, 0.015696f, -0.0488516f, 0.00357405f, 0.395393f, 0.0142406f, 0.0375136f, 0.0266987f, 0.00442581f, -0.0355697f, 0.0566785f, -0.0609618f, 0.0953531f, 0.0234361f, -0.0235014f, -0.0201052f, 0.0185904f, 0.0944014f, -0.00254259f, 0.0149094f, -0.00267577f, -0.0236442f, 0.0304207f, 0.0195184f, 0.00453831f, -0.010829f, -0.00384567f, -0.00720987f, 0.00142745f, 0.00339592f, 0.0255406f, -0.0328377f, -0.0418446f, 0.00524565f, -0.019943f, -0.00744414f, -0.0262656f, -0.00295384f, -0.012041f, 0.00168772f, -0.0393009f, -0.0333347f, -0.0127033f, -0.0399219f, -0.12722f, -0.223577f, 0.0811929f, -0.130626f, -0.0705225f, 0.174048f, 0.0435034f, -0.136602f, 0.00640297f, -0.166342f, 0.0597288f, 0.0182928f, 0.00638083f, 0.00566142f, 0.0143743f, -0.0117229f, -0.00092003f, -0.00302193f, 0.0193828f, 0.0549159f, -0.01403f, -0.0686686f, -0.00131562f, -0.0395576f, 0.0140634f, 0.00728921f, -0.0222314f, 0.0847774f, 0.00397858f, -0.037106f, 0.00703206f, 0.0217107f, 0.026982f, -0.0970178f, 0.00170535f, 0.00461989f, -0.0484043f, 0.0549405f, -0.00663961f, -0.0301618f, 0.0402775f, -0.126174f, 0.042974f, 0.00767555f, -0.0323881f, -0.0021808f, 0.00152122f, -0.0794255f, 0.00950137f, 0.00617034f, -0.186531f, 0.0667047f, 0.158624f, -0.0498641f, 0.000181888f, -0.00194408f, 0.0130678f, -0.0624929f, 0.099144f, 0.00810417f, 0.174436f, 0.0147924f, 0.00815054f, 0.0152255f, -0.0833151f, -0.072767f, -0.201512f, -0.0109339f, -0.003133f, -0.00430304f, -0.0208616f, -0.0187232f, 0.0277294f, -0.451013f, 0.0336152f, -0.00462652f, 0.00806012f, -0.000483294f, 0.0313363f, 0.0948398f, -0.0302999f, -0.00779582f, -0.0975373f, 0.0429978f, -0.0117262f, -0.00451523f, -0.0175741f, 0.0914118f, 0.0390275f, 0.00306197f, 0.0172763f, 0.0486995f, -0.0628708f, -0.00845093f, 0.00565009f, -0.0126375f, 0.0362389f, -0.0893211f, -0.0264466f,
        0.0309426f, -0.0247239f, -0.0618656f, -0.16444f, 0.0416493f, -0.0039234f, -0.0446445f, -0.0806408f, 0.0315374f, -0.0123988f, 0.0385759f, 0.0315165f, 0.00742563f, -0.0276244f, 0.013597f, -0.000546713f, -0.126003f, -0.0403999f, -0.0199147f, 0.090123f, 0.0122743f, 0.0904552f, 0.0480448f, -0.0274991f, -0.0463688f, 0.132874f, -0.0163207f, 0.00931698f, 0.00050237f, -0.034227f, 0.0273549f, 0.0257694f, 0.0545361f, -0.0196519f, -0.00616926f, 0.0252382f, 0.00394299f, 0.00503618f, -0.000107687f, -0.00739968f, 0.0155088f, -0.0271828f, 0.0136159f, -0.0184294f, 0.00419291f, -0.0705982f, 0.00832841f, -0.0455188f, 0.0203078f, -0.0104058f, -0.00448528f, 0.0346675f, 0.00227903f, 0.0283768f, 0.0146701f, 0.0238016f, -0.0041065f, -0.00951874f, -0.0656203f, 0.00289312f, -0.0280637f, 0.064775f, -0.0145084f, -0.0166982f, 0.112919f, -0.030709f, -0.08767f, 0.0231176f, -0.00683745f, 0.145201f, -0.0588483f, -0.00211676f, 0.0707442f, -0.0175353f, 0.0425204f, 0.047214f, -0.00454212f, 0.108341f, -0.0655429f, -0.0661698f, -0.00742549f, 0.0525604f, -0.00200138f, 0.0760939f, 0.0208251f, -0.0183413f, -0.019956f, 0.0497461f, -0.00312012f, -0.026077f, -0.00492334f, -0.0389153f, -0.0240003f, -0.0236527f, -0.00949685f, 0.00834218f, 0.196113f, -0.0203076f, -0.0373067f, 0.0511745f, -0.000502779f, -0.0506356f, 0.0270005f, 0.0560514f, -0.0566957f, 0.00592365f, -0.0950855f, 0.0330845f, 0.0126008f, -0.0178738f, 0.00655207f, -0.00560155f, 0.0226922f, 0.122885f, -0.0227311f, -0.0185407f, -0.024025f, 0.000734875f, -0.0501656f, 0.00259467f, -0.0401208f, -0.00270448f, 0.0298842f, -0.0449168f, -0.083653f, -0.0667249f, -0.012424f, 0.0228182f, -0.0256871f, 0.0103425f, 0.00584589f, -0.0313978f, -0.00512387f, -0.0389378f, 0.00783504f, 0.0246462f, 0.0204282f, -0.0313174f, 0.0293227f, -0.0135298f, 0.0250816f, 0.00154453f, 0.00455047f, 0.0664336f, -0.0924272f, 0.0141598f, 0.0249505f, 0.0114919f, 0.127537f, -0.0302333f, -0.0464173f, 0.0312457f, 0.0119746f, 0.00862732f, -0.0221585f, -0.00284848f, 0.014157f, 0.0253277f, 0.00495452f, 0.00886403f, 0.00389645f, -0.0347684f, -0.0039163f, 0.0218669f, -0.0417104f, 0.00547612f, -0.013528f, -0.00265715f, 0.180858f, -0.000752272f, -0.18944f, 0.0260848f, -0.000632882f, 0.0126054f, 0.0359676f, 0.0302849f, -0.0371376f, 0.0941217f, -0.0281283f, -0.0280773f, -0.011986f, -0.0406752f, 0.239648f, -0.00517518f, -0.00410975f, 0.00103368f, 0.0209206f, -0.0476301f, 0.00454544f, -0.0149667f, -0.0314583f, -0.00242636f, -0.0512553f, 0.0608112f, 0.0428258f, 0.0173526f, 0.0602241f, -0.0548611f, -0.131965f, -0.0495486f, 0.00765915f, -0.062264f, -0.000979455f, -0.0652348f, -0.147691f, -0.0231597f, 0.0251012f, -0.0946399f, 0.0277068f, -0.00621829f, 0.0313192f, 0.0259072f, 0.00394534f, -0.0118648f, 0.004981f, 0.0594206f, -0.0358001f, -0.0710233f, -0.00969833f, 0.023656f, -0.0388052f, -0.00855584f, 0.259141f, 0.0142973f, -0.00158563f, 0.0164536f, 0.0212657f, 0.00174633f, 0.0514006f, -0.00881672f, 0.0221807f, 0.0413859f, 0.0143335f, -0.163744f, 0.236609f, 0.0189168f, -0.0167902f, 0.0688642f, -0.0370002f, -0.0330411f, -0.0653769f, 0.00270779f, -0.00759605f, -0.0221796f, 0.0385442f, -0.0446415f, 0.06948f, -0.033133f, -0.0352207f, -0.0310347f, 0.00721417f, 0.0857527f, 0.00283876f, -0.115239f, 0.0347347f, -0.0365242f, 0.0587821f, 0.00664576f, -0.0273541f, -0.016766f, -0.0138301f, 0.00564337f, 0.0364023f, -0.0560315f, -0.0449002f, -0.0932135f, -0.0177926f, -0.0494535f, 0.0610707f, -0.00528969f, 0.114377f, -0.0275389f, 0.0177389f, -0.0280061f, -0.00589614f, -0.00858413f, 0.0105453f, -0.0247948f, -0.0472122f, -0.000931705f, -0.0574841f, -0.0412944f, 0.00216405f, -0.0681429f, -0.00229429f, 0.00222781f, -0.0102497f, -0.0110639f, 0.0254925f, 0.0135797f, -0.0289002f, 0.00603638f, 0.0356664f, -0.0870163f, 0.552476f, 0.0106117f, -0.025193f, -0.0567232f, 0.00731144f, -0.00597211f, 0.00564131f, -0.037914f, 0.00553956f, 0.0244306f, 0.0163081f, 0.0614898f, -0.0103462f, -0.0125773f, -0.0129543f, -0.0425792f, -0.00984468f, 0.0241087f, 0.0391885f, 0.0113726f, 0.0740247f, -0.0314575f, 0.0847706f, 0.00766129f, -0.00782563f, -0.00219977f, 0.0364213f, 0.00561357f, -0.0207095f, -0.0389947f, -0.0574235f, -0.0215928f, 0.0242519f, 0.0150763f, 0.00640004f, 0.0049859f, -0.0883498f, 0.0259088f, -0.00976872f, -0.0257561f, -0.145433f, 0.0186583f, -0.0313577f, 0.0232484f, 0.135472f, -0.0611472f, -0.0134871f, -0.0152308f, 0.0481365f, -0.000509527f, 0.0241717f, -0.0205968f, -0.0464828f, 0.00742741f, -0.0585818f, 0.0174123f, -0.032865f, 0.0399474f, 0.0189778f, 0.0185407f, -0.0144228f, 0.0195944f, 0.0105867f, 0.0108527f, 0.0318328f, -0.07468f, 0.0640258f, -0.0166149f, -0.0161666f, 0.0270572f, -0.00831346f, 0.0213354f, -0.0331297f, 0.0314013f, -0.0295451f, -0.0309544f, 0.00883464f, -0.000784053f, 0.00228157f, 0.030596f, -0.0169894f, -0.0723077f, 0.0142356f, -0.042197f, -0.0273198f, 0.0607149f, 0.0824823f, 0.0722077f, -0.0207748f, -0.0090944f, 0.0268541f, 0.0273479f, 0.00481306f, -0.00487477f, -0.0183224f, -0.0126787f, 0.0311318f, -0.0985153f, -0.0152497f, 0.00489618f, -0.0141078f, -0.0060658f, -0.000568589f, -0.032613f, 0.00976906f, -0.0462634f, -0.0259696f, -0.0786609f, -0.0153404f, 0.0249492f, 0.00292531f, -0.0255124f, 0.0202219f, 0.0304817f, -0.0177191f, -0.0135411f, -0.0064023f, 0.048916f, 0.0348483f, -0.00747575f, 0.0256531f, -0.0264167f, -0.027836f, 0.026632f, -0.0408624f, 0.0405082f, 0.0435032f, -0.0481381f, 0.0232822f, 0.0406269f, -0.104934f, 0.032984f, 0.00642478f, -0.0123055f, 0.0323379f, 0.0262914f, -0.00313157f, -0.0307961f, -0.059502f, 0.043095f, -0.0842975f, -0.0634201f, -0.0069968f, -0.0269704f, 0.0525556f, -0.0145985f, -0.026517f, 0.0287775f, -0.00225143f, 0.00998218f, -0.0208695f, 0.00038333f, -0.0179813f, 0.0299511f, -0.0270286f, -0.0215702f, 0.00986492f, -0.121571f, 0.0374826f, 0.0280122f, -0.0349332f, 0.00798409f, 0.00126605f, 0.0544963f, -0.00189064f, -0.0770879f, -0.00792704f, 0.0613617f, 0.0133352f, 0.0303873f, -0.000380032f, 0.0189077f, -0.0194632f, -0.00659714f, -0.0571043f, 0.041608f, -0.0141942f, 0.012823f, 0.00537086f, 0.000970999f, 0.0332154f, 0.0570762f, -0.0137126f, 0.0101087f, -0.00108052f, -0.0265809f, -0.0247709f, -0.00362676f, -0.0148946f, 0.013131f, -0.00308769f, -0.158096f, 0.00257066f, -0.0143705f, 0.0888035f, 0.00916709f, 0.00514034f, -0.0227268f, 0.134988f, -0.0492885f, 0.0022784f, -0.0144922f, 0.0256463f, 0.0246127f, -0.0242015f, -0.0270194f,
        0.0236487f, -0.00133765f, -0.023996f, 0.0121123f, 0.0473768f, -0.0229827f, 0.0620781f, 0.0348273f, 0.0118778f, -0.0358558f, -0.00418959f, 0.026328f, 0.00159447f, -0.0285201f, 0.0242085f, 0.024281f, -0.120022f, 0.00322402f, -0.00124464f, -0.00395719f, 0.00586048f, 0.0264264f, 0.0202582f, -0.0172882f, 0.0167585f, 0.00926656f, 0.00103096f, 0.00249462f, 0.00288184f, -0.00771514f, 0.0255329f, 0.0516628f, -0.0170072f, -0.00388561f, -0.00997277f, 0.0355019f, 0.000978238f, -0.144348f, -0.00646585f, -0.013882f, 0.033804f, -0.0377087f, 0.00771159f, -0.0061665f, 0.0237085f, -0.0122598f, 0.0771705f, -0.0542605f, -0.0292168f, -0.0110855f, 0.00780249f, -0.0262439f, -0.0170252f, 0.0232333f, 0.0221474f, -0.000682905f, 0.0456239f, 0.00516233f, -0.0356498f, 0.0433573f, -0.0725911f, 0.122393f, -0.000836771f, 0.0154195f, -0.00217232f, -0.0458872f, 0.0576701f, 0.0347757f, 0.00437707f, 0.0167836f, -0.024089f, 0.00395376f, 0.0226754f, -0.000325613f, -0.0119747f, 0.0166885f, 0.0133881f, -0.00825686f, -0.0115485f, -0.0256805f, -0.013069f, 0.029991f, -0.0104672f, 0.0468771f, 0.018202f, -0.0499781f, -0.0150365f, 0.0351706f, 0.000881884f, 0.0257364f, -0.00567146f, -0.0125245f, -0.00638529f, 0.00949407f, -0.00206895f, -0.00294736f, -0.00599403f, 0.0100478f, -0.0708312f, 0.0164853f, -0.00509979f, -0.0820398f, 0.00301894f, -0.011352f, -0.103304f, 0.0361376f, -0.00276168f, 0.0140668f, 0.0182486f, -0.0224722f, 0.00670642f, -0.00173934f, -0.0763404f, 0.00545386f, -0.0451032f, 0.258199f, -0.000526159f, -0.00244376f, -0.0070213f, 0.0136966f, 0.00651444f, 0.00336226f, 0.0129456f, -0.00535145f, -0.0337439f, -0.0488545f, 0.0363396f, -0.000131419f, -0.0442874f, -0.00468587f, -0.00406768f, -0.0170205f, -0.0192772f, -0.00277597f, 0.0212662f, 0.0767458f, -0.0198272f, 0.00671115f, 0.00387314f, -0.00222632f, 0.017668f, -0.0152864f, -0.00217823f, -0.0302261f, 0.0201784f, 0.00912841f, 0.0418803f, 0.00397826f, -0.0171634f, 0.0562426f, -0.00595202f, 0.0317872f, 0.00277863f, -0.0198806f, -0.0105047f, -0.0078311f, -0.00416702f, 0.0284072f, 0.00135271f, 0.00845078f, 0.0125683f, -0.00724979f, 0.0567957f, 0.0255109f, 0.002417f, 0.0114722f, -0.0229208f, 0.00542141f, 0.000680912f, -0.0124263f, -0.0973681f, 0.0429572f, -0.00896565f, 0.00102447f, 0.0209145f, 0.0365617f, 0.00698999f, 0.0611891f, -0.0021814f, -0.00791606f, 0.0636013f, -0.0503155f, 0.041678f, -0.00722059f, -0.00547887f, 0.00243705f, -0.0177814f, -0.12321f, 0.0569086f, -0.00487058f, 0.0123446f, 0.0015868f, -0.0272469f, 0.0180903f, 0.0104843f, 0.0105209f, 0.00808024f, -0.0662313f, -0.0499085f, -0.0297908f, 0.00678693f, 0.0158422f, -0.0149847f, -0.212685f, -0.029142f, -0.0216139f, 0.0197027f, -0.00509483f, 0.0406666f, -0.00101148f, 0.0137954f, 0.0292058f, 0.0261623f, 0.0879647f, -0.0120199f, 0.0276628f, -0.00208332f, 0.00630364f, -0.00283301f, 0.0313885f, 0.00132789f, 0.00430711f, 0.131565f, 0.00856252f, -0.0451589f, 0.0151607f, -0.00609563f, 0.104563f, 0.0503204f, -0.00188153f, -0.00152094f, 0.0331939f, -0.0268272f, -0.0720271f, 0.0120254f, 0.00428272f, -0.010781f, -0.0235618f, -0.0599427f, -0.0128298f, -0.039684f, 0.0124311f, -0.00907946f, -0.0219339f, -0.00574204f, 0.00290369f, -0.0397143f, -0.0306637f, 0.0046412f, -0.102802f, 0.02052f, 0.0177221f, -0.000307451f, -0.663219f, -0.00099111f, -0.00863413f, -0.0648291f, 0.141571f, -0.0264896f, -0.00967159f, -0.0105556f, 0.00667919f, 0.019933f, -0.0081883f, -0.0256497f, -0.0425081f, -0.00260382f, -0.00437219f, 0.0181059f, 0.0588014f, -0.0156841f, -0.0992774f, 0.0577409f, -0.0112435f, 0.0118955f, -0.01259f, -1.68039e-05f, -0.0231843f, -0.0715207f, 0.00562568f, 0.00659099f, -0.00432696f, 0.0402245f, -0.0132643f, 8.8306e-05f, 0.00698941f, -0.0695019f, -0.0112349f, 0.0696259f, -0.142201f, -0.0227633f, -0.019462f, -0.0518398f, -0.0213576f, 0.0148991f, 0.0344155f, -0.0131575f, -0.012708f, -0.00177817f, -0.00639755f, -0.000887201f, -0.0257106f, -0.0247181f, 0.00548285f, 0.0290425f, 0.122557f, -0.00347772f, 0.0268244f, -0.00612725f, -0.0196236f, -0.0472946f, 0.00890478f, 0.000844572f, 0.0154442f, 0.024701f, -0.0306896f, 0.0231992f, 0.0425512f, -0.0302086f, 0.0319046f, 0.0310391f, -0.00796268f, -0.0411025f, 0.00749199f, -0.0374908f, -0.0108962f, 0.0293042f, 0.00369268f, -0.0138972f, -0.00285899f, -0.0473339f, 0.00105261f, 0.0269907f, -0.0314717f, -0.0538936f, 0.0837861f, -0.0145771f, 0.0345362f, 0.222726f, -0.034146f, -0.0154113f, 0.0519213f, 0.0351403f, -0.0609869f, 0.0181544f, -0.0165051f, 0.00702428f, -0.0109979f, -0.00444243f, -0.018915f, -0.027162f, 0.00253407f, 0.0133815f, -0.000469394f, 0.109107f, 0.0153356f, 0.00683112f, 0.0128685f, 0.0282692f, -0.0384653f, 0.000389417f, 0.106818f, 0.0799349f, 0.0567321f, 0.0479257f, 0.00394279f, -0.00575818f, -0.575371f, -0.0118667f, 0.00356253f, -0.0399865f, -0.0217626f, -0.019511f, 0.0108772f, 0.0134627f, -0.000487889f, -0.00162015f, -0.0268957f, 0.0158162f, 0.0124589f, 0.0514896f, 0.0391116f, -0.02102f, 0.0289451f, -0.0162062f, 0.0295524f, 0.0240599f, 0.00653552f, -0.0296798f, -0.0614426f, 0.00678693f, -0.0126935f, -0.0259306f, -0.0270236f, -0.005202f, -0.027559f, -0.00571665f, 0.01303f, -0.0176816f, 0.00828625f, -0.0159388f, 0.016197f, -0.0685197f, 0.0359586f, -0.0149305f, -0.0100357f, -0.054005f, 0.0405895f, -0.0436483f, -0.0196033f, 0.0205626f, 0.0601753f, 0.00745636f, 0.00526461f, 0.00770411f, -0.00536197f, -0.0196271f, -0.00742883f, 0.0673765f, 0.0225239f, 0.0330661f, -0.0197954f, 0.0635232f, -0.00196483f, -0.0160432f, 0.0274051f, 0.0249642f, -0.0215083f, 0.00376016f, 0.0484418f, -0.0339058f, -0.00930553f, 0.000391001f, 0.0489547f, 0.00680175f, 0.0121302f, -0.0159317f, -0.00746274f, 0.00762586f, 0.0151285f, -0.00984925f, 0.00967698f, -0.063813f, -0.00191317f, -0.0225768f, -0.0460198f, 0.0129389f, 0.022693f, -0.0331679f, -0.0252172f, 0.0152612f, -0.0615063f, 0.00776267f, 0.0890267f, -0.0218608f, 0.0164835f, -0.048754f, 0.0158734f, 0.00247796f, -0.0340838f, 0.0199824f, 0.0422744f, 0.00495236f, 0.00733676f, -0.693422f, -0.057195f, -0.042145f, -0.0894016f, 0.00573138f, 0.00168211f, -0.00815092f, 0.1004f, -0.00830388f, 0.0212194f, 0.00796229f, 0.0182782f, -0.00677567f, -0.0025772f, -0.0141583f, -0.0503938f, 0.00933939f, -0.0440368f, -0.0650577f, -0.0133163f, -0.0150479f, -0.128004f, -0.025883f, -0.0142512f, 0.0267747f, 0.0603829f, 0.0616747f, 0.00518816f, 0.0353825f, -0.0136665f, -0.0116953f, -0.0117363f, -0.00988685f, 0.0161024f, -0.0164802f, 0.0120735f,
        0.0115264f, 0.00956785f, -0.0348965f, -0.0115787f, 0.0441999f, 0.0345045f, 0.0134386f, -0.0337335f, -0.00245127f, -0.0610053f, 0.0043896f, 0.0019506f, 0.013525f, -0.0545739f, 0.0306072f, 0.105704f, -0.0610636f, 0.0184838f, -0.0121108f, -0.00898275f, 0.0264786f, 0.0351719f, 0.00565877f, -0.00984551f, 0.0349376f, 0.0065558f, 0.000771663f, 0.000747164f, 0.00623147f, -0.0100182f, 0.0147877f, 0.027002f, -0.0082708f, -0.00312388f, -0.031057f, 0.0352335f, 0.0102762f, -0.136548f, -0.00137814f, -0.0245331f, 0.0302073f, -0.050357f, -0.0055813f, -0.0035066f, 0.0159663f, -0.00413293f, -0.0220518f, -0.0378098f, -0.000528503f, -0.00883574f, -0.0160642f, -0.0976056f, -0.00949359f, 0.0667935f, 0.0152671f, -0.00275173f, -0.00305567f, -0.00027522f, -0.0358676f, 0.0613587f, -0.0621408f, 0.0603126f, -0.00382261f, -0.0162797f, 0.0627967f, -0.0338104f, 0.019684f, 0.0723154f, 0.0405459f, 0.0150282f, 0.0116941f, 0.0159087f, 0.0423308f, 0.000188638f, -0.0151563f, 0.0213552f, 0.0260785f, -0.000634076f, -0.00666879f, -0.0143571f, -0.0154005f, 0.0452614f, -0.0241995f, 0.00760913f, 0.00565907f, -0.0146403f, -0.00882357f, 0.109466f, 0.000185842f, 0.0530813f, -0.0167083f, -0.0132453f, 0.00510363f, 0.000928611f, -0.0231941f, -0.00849421f, -0.0127253f, 0.0143131f, -0.104331f, 0.0150856f, -0.0115339f, -0.0400927f, -0.00650179f, 0.00782663f, -0.0161432f, 0.00612369f, -0.0368485f, 0.0320765f, -0.000285285f, -0.0252538f, 0.00567933f, -0.00326235f, -0.0118118f, -0.0067807f, -0.0626707f, 0.0314245f, -0.00367115f, 0.0034559f, 0.00094028f, 0.012767f, -0.0376215f, -0.0102952f, 0.0236869f, 0.00184345f, -0.0418395f, -0.0542331f, -0.00655869f, -0.00491183f, -0.0167015f, -0.0135059f, -0.0126727f, -0.0262544f, -0.0235505f, -0.00927455f, 0.044421f, 0.0340354f, 0.0544527f, 0.0133111f, 0.00308665f, 0.00078136f, -0.0023735f, -0.0141342f, 0.00124783f, -0.0175074f, 0.0506524f, 0.0344784f, 0.016513f, 0.00434411f, -0.0224391f, 0.0865785f, -0.00372209f, -0.0103298f, -0.00164323f, -0.0143697f, -0.0125625f, -0.00602005f, -0.00435671f, -0.0097799f, -0.00277924f, 0.0124438f, 0.00866435f, 0.00456806f, 0.032294f, 0.00501145f, 0.0381001f, 0.0142146f, -0.0373586f, -0.0278584f, -0.0268059f, -0.0109542f, 0.0129881f, -0.0289077f, -0.00849425f, 0.00391238f, 0.0105073f, 0.0449334f, 0.00855353f, 0.0402285f, -0.00646413f, -0.00671409f, 0.013527f, -0.0528845f, 0.0319318f, -0.0113917f, -0.0113392f, -0.000316065f, 0.0412851f, -0.0162739f, 0.0137208f, -0.0163712f, 0.0349673f, 0.00457418f, -0.0198638f, 0.0765183f, -0.001026f, 0.0113388f, 0.00846672f, 0.0122229f, -0.0401006f, -0.00219702f, 0.00703645f, 0.0321573f, 0.000362714f, -0.24312f, -0.014646f, -0.00614563f, 0.0187569f, -0.00394876f, 0.0243838f, -0.00188284f, 0.0050112f, 0.0221267f, -0.00302741f, 0.0435336f, -0.0226377f, 0.0262879f, 0.0155468f, 0.0279725f, -0.00188527f, -0.00564561f, -0.00020769f, 0.0150204f, 0.13116f, 0.021348f, 0.00731956f, -0.0343524f, 0.00212442f, 0.0352829f, 0.526485f, -0.00325235f, -0.00250349f, 0.0161844f, -0.0453576f, -0.0154224f, -0.0407768f, 0.0031079f, -0.00879997f, 0.00831367f, -0.0461003f, -0.0249753f, -0.0173187f, 0.0510597f, 0.0221946f, -0.0149577f, 0.000957178f, 0.0111411f, 0.00876051f, -0.0220329f, -0.0046637f, -0.020372f, 0.00369127f, 0.039286f, -0.00385722f, 0.0115072f, -0.00474474f, -0.0141273f, -0.19162f, -0.0187427f, -0.00145075f, -0.00458649f, -0.00136821f, 0.0037382f, 0.0102019f, -0.0101349f, -0.0303892f, -0.697959f, -0.00391341f, -0.00169856f, 0.0454146f, -0.0300301f, -0.0387779f, -0.0249505f, -0.0183996f, -0.00471838f, -0.00533851f, 0.000305908f, -0.00737827f, -0.0143906f, -0.0612462f, 0.0117793f, -0.0296389f, -0.0045701f, 0.0974987f, -0.0222056f, -0.00917552f, 0.00540695f, 0.376f, -0.0369584f, 0.0818413f, -0.0806179f, -0.0591828f, -0.0292424f, 0.0175326f, -0.0141385f, 0.01833f, 0.0209717f, -0.0198613f, -0.0303378f, -0.00184021f, -0.095508f, 0.00121903f, 0.00795399f, -0.0660669f, -0.000692821f, 0.00370955f, 0.140168f, -0.000690335f, 0.0085036f, -0.0224978f, 0.0989872f, -0.103726f, -0.00133824f, 0.00176511f, 0.0226218f, 0.00723803f, -0.0136401f, 0.0136266f, 0.00908615f, -0.0421018f, -0.0535609f, -0.0230947f, -0.0338358f, -0.00108633f, -0.0356084f, -0.109221f, -0.014515f, 0.0077523f, 0.0139792f, -0.0248496f, -0.023008f, -0.0472426f, 0.0865438f, 0.000595621f, -0.0451802f, -0.0395005f, 0.0493621f, -0.00124904f, 0.0988936f, 0.0572095f, -0.0729679f, -0.00415711f, 0.161504f, -0.00328739f, -0.0133308f, 0.00799106f, -0.0163052f, -0.0209516f, 0.00308542f, -0.0129289f, -0.0510538f, -0.0122714f, -0.0362058f, 0.0683402f, -0.0126313f, 0.0263825f, 0.0168551f, 0.00470125f, 0.0204198f, 0.0145374f, -0.021401f, 0.00460656f, 0.085484f, 0.0781075f, 0.0251125f, 0.00791536f, -0.0189591f, -0.0431845f, 0.051558f, 0.017842f, 0.36608f, -0.0343333f, -0.0303445f, -0.0115494f, 0.0530173f, 0.0165506f, -0.0235855f, -0.052452f, -0.00888096f, 0.0221193f, 0.0386185f, 0.0353902f, 0.0246971f, -0.0122489f, 0.0512722f, 0.00400143f, 0.0255521f, 0.00548785f, 0.00233302f, -0.0253462f, -0.0966852f, 0.00378993f, 0.00350757f, -0.0310213f, -0.0279353f, -0.00233223f, -0.0220107f, 0.00163079f, -0.00717164f, 0.00659987f, -0.00608499f, -0.02305f, 0.00402512f, -0.32546f, 0.0706807f, 0.0274278f, 0.0267394f, -0.00604822f, 0.0361692f, -0.0515999f, 0.0369351f, 0.0124044f, 0.0716815f, 0.0053833f, 0.00673388f, 0.0250085f, -0.000686182f, -0.00550432f, -0.00231397f, 0.00181825f, 0.022164f, 0.0330005f, -0.00140523f, 0.0463948f, -0.0278037f, -0.0318544f, 0.0275073f, 0.0620945f, -0.0128747f, 0.0329174f, 0.0206743f, -0.0352932f, -0.00835452f, 0.0248623f, 0.119621f, -0.0292978f, -0.0132096f, -0.0302576f, -0.0178306f, 0.0209123f, 0.0229405f, -0.0236861f, 0.00108116f, -0.0799521f, 0.00532662f, 0.0127616f, -0.00190055f, 0.00847102f, 0.00451121f, -0.0637118f, -0.0302129f, 0.0119081f, -0.117328f, -0.00946109f, 0.0605782f, -0.0390624f, 0.0192556f, -0.0170363f, 0.0300991f, 0.0444662f, 0.0422317f, 0.0170539f, 0.0504948f, 0.0270332f, 0.00916911f, 0.0242343f, 0.00898315f, -0.0158267f, -0.0475899f, 0.0175909f, -0.000817633f, -0.0176624f, 0.0975135f, -0.00854145f, 0.0155055f, 0.00762038f, 0.0229743f, -0.0525053f, -0.0149161f, -0.0367894f, -0.104801f, 0.013039f, -0.120883f, -0.0715135f, -0.0193206f, 0.0158965f, -0.0748989f, -0.120509f, -0.0506567f, 0.0147239f, 0.107749f, 0.0659703f, 0.0220761f, 0.0242295f, 0.0180054f, -0.0111281f, -0.0171504f, -0.014431f, 0.083154f, 0.0241038f, 0.0115941f,
        0.0112054f, -0.208447f, -0.0871743f, -0.0362684f, -0.0110118f, 0.068481f, 0.0322887f, -0.0375058f, -0.0130676f, -0.101841f, 0.0479009f, 0.0459907f, 0.00208143f, -0.0880017f, 0.0160549f, -0.0533964f, -0.0336657f, -0.000403741f, 0.0274574f, 0.00649047f, -0.0278283f, -0.0254132f, 0.0467184f, -0.0375531f, 0.127941f, 0.0291329f, 0.00155753f, 0.00199031f, 0.0183402f, 0.155697f, 0.0500429f, 0.00407514f, 0.0229933f, -0.00482785f, -0.0220735f, 0.0390895f, -0.0863406f, -0.132777f, 0.00204372f, -0.0069423f, 0.0260759f, -0.031759f, -0.00107891f, -0.0218382f, 0.00464639f, -0.00370248f, 0.00721869f, -0.0152541f, -0.00113688f, -0.00731756f, -0.0459436f, -0.0122795f, -0.0212339f, 0.072953f, 0.0268922f, -0.00254329f, -0.00535364f, 0.0200235f, -0.019393f, 0.00740422f, -0.0515143f, 0.0410708f, -0.00789718f, -0.0633389f, 0.0544137f, -0.0580859f, 0.0325159f, -0.015541f, 0.0178216f, 0.289658f, -0.0234133f, -0.0074536f, 0.0255261f, 0.00291012f, -0.0219596f, 0.0246941f, -0.00560577f, 0.00899517f, 0.00914874f, -0.0254892f, -0.0521876f, 0.0629406f, -0.00645591f, 0.111561f, 0.0122516f, -0.0106223f, -0.0132192f, -0.0819937f, 0.0132221f, -0.00695472f, -0.0207924f, -0.0723628f, 0.0495887f, -0.0359372f, -0.04756f, -0.0288064f, -0.08486f, 0.285901f, -0.0527237f, 0.0401743f, 0.00317573f, -0.00912604f, -0.00509804f, -0.019646f, -0.0133663f, 0.00250147f, 0.00489291f, 0.017901f, 0.117288f, -0.0253837f, 0.0201622f, -0.0127631f, -0.000326688f, -0.0153231f, -0.0756543f, 0.113002f, -0.0181392f, -0.00927301f, 0.0726324f, 0.00722584f, -0.0730271f, 0.0245927f, -0.102462f, 0.0356965f, -0.0606429f, -0.0444952f, -0.0166311f, 0.00795211f, -0.00189904f, -0.0158499f, -0.0204771f, -0.0472794f, -0.0079858f, -0.0501545f, 0.102751f, 0.0584957f, 0.0372233f, 0.00862791f, 0.00449617f, -0.0237138f, 0.00679621f, -0.0152089f, -0.00387291f, -0.126512f, -0.0284672f, -0.0684034f, 0.0303137f, -0.0162955f, -0.0581197f, -0.220276f, -0.00417518f, -0.0689113f, -0.017655f, -0.0224894f, 0.0357768f, 0.0133865f, 0.022937f, 0.0472434f, -0.00953042f, -0.0159915f, 0.00998823f, 0.00600883f, 0.0533401f, 0.194183f, 0.477756f, 0.0191196f, 0.0227464f, -0.00284643f, -0.13471f, 0.0769816f, 0.01241f, -0.0497929f, -0.0935632f, 0.0292851f, 0.0178327f, 0.104592f, -0.0467304f, -0.00100124f, -0.0401962f, -0.0224538f, -0.00678469f, -0.073481f, 0.227438f, -0.00830996f, 0.073789f, -0.0239749f, 0.154952f, -0.0544236f, 0.0156297f, 0.19281f, 0.0326588f, -0.00926173f, -0.0288493f, 0.0228173f, 0.0186095f, 0.0415022f, 0.0290895f, -0.00247426f, -0.0898812f, 0.0274265f, 0.0393059f, 0.0222607f, 0.019877f, -0.150684f, -0.262853f, -0.0894445f, -0.0205114f, -0.00142168f, 0.126473f, -3.85201e-05f, 0.0356633f, 0.0269576f, 0.0157574f, -0.0432543f, 0.0279592f, 0.024804f, -0.0267448f, 0.0191669f, -0.0040675f, 0.0139007f, 0.00963236f, -0.0110146f, 0.137714f, 0.0166686f, 0.0200946f, -0.0611695f, -0.0639973f, 0.0055134f, 0.042783f, 0.0271225f, -0.0468356f, 0.0247138f, 0.0103724f, 0.00932251f, -0.0140851f, 0.0358128f, -0.0059887f, 0.0386251f, -0.00545864f, 0.0596616f, -0.0379678f, 0.0116168f, -0.0113317f, -0.0299328f, 0.0217457f, 0.0063076f, -0.00526829f, -0.012835f, 0.0163333f, -0.0390477f, 0.0108823f, 0.127479f, -0.00949771f, 0.000669599f, -0.00832522f, -0.00771118f, -0.554012f, -0.259737f, 0.00827122f, -0.000538992f, 0.0152035f, 0.05717f, 0.00494831f, -0.0414577f, 0.0166355f, 0.0400496f, -0.0114314f, -0.0214246f, 0.00867137f, -0.0404191f, -0.0166356f, 0.0428265f, 0.0146152f, 0.00234592f, -0.0864799f, 0.0226774f, 0.00508847f, 0.0203778f, -0.0583453f, -0.00666855f, -0.127756f, -0.00862127f, 0.0452925f, -0.0831513f, -0.00326817f, 0.00995622f, 0.116901f, -0.0877858f, 0.112396f, -0.102312f, -0.105516f, -0.0259396f, 0.00757632f, 0.00122858f, 0.0103624f, 0.0457345f, -0.0242102f, -0.0583132f, -0.012498f, -0.313943f, 0.0069556f, -0.0319396f, 0.0172862f, -0.00853725f, 0.0116005f, 0.125311f, 0.00419865f, 0.0476964f, 0.00896339f, 0.00977134f, -0.0925261f, 0.0156905f, -0.018496f, 0.0196972f, 0.0157389f, -0.00196949f, 0.0145061f, -0.0606428f, -0.0694258f, 0.0709404f, 0.00871243f, 0.00455373f, -0.00558034f, -0.0824924f, 0.0011513f, 0.0384797f, 0.00638306f, 0.00363507f, -0.0606946f, -0.0774373f, -0.020545f, 0.0937525f, -0.00557294f, -0.0987101f, -0.0864387f, -0.0108511f, -0.0149365f, 0.0481765f, 0.036998f, -0.112909f, -0.00983293f, 0.135054f, 0.071086f, 0.019128f, 0.00687174f, -0.00651517f, -0.0349884f, -0.00583317f, -0.0110052f, 0.0398168f, -0.0141334f, -0.0344924f, -0.00134893f, -0.0270122f, -0.000114596f, 0.0220215f, -0.0321631f, 0.0329176f, 0.0261847f, 0.170964f, 0.0083325f, -0.0209986f, -0.00422142f, 0.00124639f, -0.000368193f, 0.00871341f, -0.0488562f, 0.0170233f, 0.0236273f, -0.0163899f, -0.120393f, -0.151225f, -0.00206636f, 0.105974f, -0.00312998f, 0.0290657f, -0.014112f, -0.0107348f, 0.032648f, 0.026346f, 0.0817057f, 0.0319139f, 0.00208954f, 0.0860523f, 0.0385837f, 0.115668f, 0.0399777f, 0.00339539f, -0.00545459f, 0.0598012f, 0.00298756f, 0.0148942f, -0.0227489f, -0.0139737f, -0.00473305f, -0.0326132f, -0.0229166f, 0.0207593f, 0.00277288f, -0.00719371f, -0.0202886f, -0.00475025f, 0.00617697f, 0.0162748f, 0.124789f, 0.0101917f, -0.0547861f, 0.0414249f, -0.0484098f, -0.0963767f, 0.0484124f, -0.00538394f, 0.00789277f, -0.0102249f, 0.104348f, 0.00192805f, 0.00647828f, -0.0461272f, -0.00422982f, 0.0325315f, 0.0211869f, 0.0120108f, 0.0362735f, -0.100353f, -0.106846f, 0.0453949f, 0.108593f, -0.00433587f, 0.0477131f, -0.011734f, -0.00221842f, -0.0186096f, 0.0176472f, 0.0535756f, -0.00753715f, -0.00288954f, 0.0351324f, -0.000401414f, 0.0260439f, 0.0353749f, -0.0214858f, 0.000924544f, 0.000524206f, 0.0411247f, -0.0106592f, 0.0429043f, 0.0430829f, 0.029413f, -0.00455873f, -0.0157798f, 0.0105745f, -0.10904f, -0.0209529f, -0.0605732f, 0.0151213f, 0.0293344f, -0.030329f, 0.0388919f, 0.0830521f, -0.0636824f, 0.0834155f, -0.0213396f, -0.0029129f, 0.0130443f, 0.0276463f, 0.0168069f, 0.0131674f, 0.00569299f, 0.0319241f, -0.215148f, -0.080939f, 0.033579f, -0.0317194f, 0.0329596f, -0.000564258f, 0.0486169f, -0.0763688f, -0.0114993f, -0.0945774f, -0.0518434f, 0.0208386f, -0.059539f, -0.109562f, 0.0544319f, -0.0264226f, -0.090276f, -0.0850728f, -0.0434345f, -0.00627017f, 0.0531473f, 0.214103f, -0.0206121f, 0.0996398f, 0.155764f, -0.00199676f, -0.0115997f, -0.0355156f, 0.113538f, 0.0365645f, 0.0733962f,
        -0.00286558f, -0.000197294f, -0.00342685f, 0.0153883f, -0.0289065f, -0.00108885f, 0.0387835f, 0.00578341f, -0.0262422f, 0.0582573f, -0.0156079f, 0.00150583f, -0.0362458f, 0.0104373f, 0.0113533f, -0.0386981f, -0.00206408f, 0.00844815f, -0.053396f, -0.00516133f, -0.0270117f, -0.27301f, 0.0846544f, -0.00616813f, 0.0871727f, 0.00543487f, 0.00184716f, 0.000983837f, 0.0179262f, 0.0818094f, 0.00281717f, 0.000740774f, -0.19657f, -0.00320105f, -0.00607788f, 0.0319331f, -0.0235907f, -0.0270345f, -0.00849474f, -0.0110374f, -0.0746362f, -0.00860617f, -0.0237307f, -0.00439848f, 0.0264778f, -0.00958103f, 0.019552f, -0.00263648f, 0.0296515f, -0.00113696f, -0.124069f, -0.0114847f, 0.00308178f, 0.0198766f, 0.0900992f, 0.00727182f, -0.0940811f, 0.0216224f, -0.0459814f, 0.017883f, -0.0572063f, 0.0627439f, -0.00563362f, -0.0648773f, -0.00153448f, -0.0913932f, 0.0230685f, 0.00374598f, 0.034431f, 0.112274f, -0.326763f, 0.0153011f, 0.00631464f, 0.00570424f, -0.0595568f, -0.0114755f, -0.0054627f, -0.000223818f, -0.00287856f, -0.0252934f, -0.0108832f, 0.0186247f, -0.0140906f, -0.0239924f, -0.0332168f, -0.00818134f, -0.0261136f, 0.0112761f, -0.0570478f, -0.0484226f, -0.00280576f, -0.140804f, -0.0192205f, 0.0193394f, 0.0043392f, -0.0096851f, -0.0238295f, 0.0496011f, -0.00870349f, 0.0271804f, 0.00735628f, 0.00931979f, -0.000362012f, -0.00038859f, 0.098518f, -0.0510564f, -0.00233872f, 0.00517725f, 0.0101231f, -0.0331861f, 0.0441328f, -0.00924161f, -0.0059294f, -0.0159056f, -0.0810096f, 0.203707f, 0.00935022f, 0.00920423f, 0.0427866f, 0.0270535f, -0.0613705f, 0.0281747f, -0.0292151f, -0.011845f, -0.560809f, -0.0430764f, 0.0249193f, 0.001065f, 0.0495798f, -0.00604974f, -0.0115863f, -0.0841969f, -0.0400231f, -0.0234006f, -0.099013f, 0.0434646f, 0.000907694f, 0.0125445f, 0.0118042f, -0.00467421f, 0.00886041f, 0.0296945f, 0.0324396f, -0.0114072f, 0.0988003f, 0.00847453f, 0.0464346f, -0.00464305f, -0.0289332f, 0.0590643f, -0.0350208f, -0.0899201f, -0.0159029f, -0.0648027f, 0.00696909f, -0.00101221f, 0.0140877f, -0.0855302f, -0.000846032f, -0.0256277f, 0.00854884f, -0.00292961f, 0.209544f, 0.0872828f, 0.0246488f, 0.0291603f, -0.0784974f, 0.00920441f, -0.011242f, -0.0297102f, 0.0152799f, -0.0428288f, -0.0651387f, 0.0138869f, 0.0139815f, 0.0836656f, 0.0361113f, -0.0635471f, -0.0160178f, -0.0220017f, 0.234027f, -0.0400384f, 0.186927f, -0.0295061f, 0.00130944f, -0.0287178f, -0.0214042f, -0.0285818f, 0.0222618f, -0.00368823f, 0.0601194f, -0.0188088f, -0.0146725f, 0.0157483f, 0.21603f, 0.056817f, -0.20685f, -0.0254415f, 0.00525571f, 0.00219887f, 0.0530388f, 0.0607272f, 0.0061378f, -0.113869f, -0.16334f, -0.0464161f, -0.00694523f, -0.00488537f, 0.0286918f, 0.00290496f, 0.178755f, 0.0109929f, 0.110835f, -0.0642967f, -0.0333608f, 0.00169389f, -0.00546941f, 0.00973807f, -0.00576067f, -0.0205257f, 0.0511577f, -0.0266243f, 0.109812f, 0.0471989f, 0.0996845f, 0.0135877f, -0.0794984f, 0.0649346f, 0.0303168f, -0.0011697f, 0.00521801f, 0.0626395f, -0.00297682f, 0.0266726f, -0.000223535f, 0.0116355f, -0.0108245f, 0.000611158f, 0.00728507f, 0.0239288f, -0.00188282f, 0.0150957f, -0.040548f, -0.0589448f, 0.0328252f, -0.0915972f, 0.0805046f, -0.00811939f, 0.0772469f, -0.0716012f, 0.000604462f, 0.047583f, 0.0334997f, -0.000381467f, -0.00726828f, 0.00027943f, -0.0427843f, -0.0568598f, 0.0147649f, -0.00348073f, 0.00288838f, 0.00979242f, -0.00538436f, -0.024106f, 0.00541673f, 0.00529046f, -0.00278852f, -0.0222607f, -0.00626747f, 0.0973789f, -0.0795939f, 0.105127f, -0.337742f, 0.0172115f, 0.00255328f, -0.0330435f, 0.0063678f, 0.0471297f, -0.050865f, -0.00217128f, 0.0139913f, -0.00278459f, 0.0452206f, -0.0122722f, 0.00537665f, 0.0068003f, -0.0241691f, -0.00537261f, 0.00198657f, 0.0288662f, -0.0673232f, -0.00391073f, 0.0160158f, -0.0148616f, 0.00889894f, 0.0278599f, -0.0259723f, -0.0464762f, -0.0699778f, 0.0855682f, -0.00447207f, -0.105144f, -0.000995281f, -0.0146742f, -0.49647f, 0.0685417f, -0.000740646f, 0.0278313f, -0.00761982f, 0.0475931f, -0.0645097f, 0.119236f, -0.0570179f, 0.00915969f, 0.0156965f, 0.101129f, -0.0274397f, 0.0317f, 0.435965f, 0.0895423f, 0.0228896f, 0.0537683f, -0.0312062f, -0.0316729f, 0.00405423f, -0.00417011f, 0.053186f, 0.0124111f, -0.0636419f, -0.059223f, 0.00212677f, -0.00180764f, -0.0184438f, -0.00539991f, -0.0216965f, -0.0297828f, -0.00665945f, 0.0659594f, 0.109878f, -0.0859683f, -0.0195527f, 0.0856906f, 0.113261f, 0.0901811f, 0.00573377f, 0.0357797f, -0.0261576f, 0.0127095f, 0.00452054f, 0.0160191f, 0.0674667f, -0.0187489f, 0.00896214f, -0.00895184f, 0.388793f, 0.0155203f, -0.206128f, -0.0134212f, 0.0159576f, 0.240592f, -0.0244503f, 0.0595618f, 0.0056212f, -0.0505254f, 0.160077f, 0.0021605f, 0.111341f, -0.664956f, 0.031356f, -0.00658282f, -0.431486f, -0.0241319f, -0.437714f, 0.0186697f, 0.0143805f, -0.0139802f, -0.00777148f, 0.0223012f, -0.0458929f, 0.0103136f, 0.0203269f, -0.0121667f, -0.00358236f, -0.0347832f, 0.0310102f, 0.0940264f, 0.0402878f, 0.0779475f, 0.085935f, 0.0506573f, 0.0125433f, 0.00945608f, 0.00711064f, -0.0157027f, -0.00267093f, -0.0460969f, 0.00133153f, 0.0510218f, 0.0568231f, 0.00654478f, -0.0148599f, -0.00556127f, 0.0984337f, 0.0012008f, 0.0401073f, -0.00218267f, -0.0913605f, 0.0250143f, 0.0269926f, -0.00189873f, 0.145338f, -0.0106285f, 0.128684f, 0.0182833f, -0.0104387f, 0.058272f, 0.054818f, -0.0204594f, 0.0514151f, -0.0114196f, 0.0121938f, -0.0135972f, 0.00423344f, 0.0268584f, -0.0233103f, 0.0149913f, 0.00556167f, 0.175006f, 0.0460865f, -0.0531133f, -0.00530817f, 0.00775018f, -0.00568381f, 0.00309299f, 0.00404426f, 0.0611169f, 0.04162f, 0.0620172f, 0.0113454f, 0.0556293f, -0.000326539f, -0.0136839f, -0.00373327f, 0.0962103f, -0.0169842f, 0.0247842f, 0.0442757f, 0.0244144f, -0.0176649f, -0.00554654f, -0.0050203f, -0.0177601f, -0.02368f, 0.0243078f, -0.0571087f, 0.0184628f, -0.0841841f, 0.0331607f, 0.0279732f, -0.0822138f, 0.0293232f, -0.0722001f, 0.0163439f, 0.0191851f, 0.414194f, 0.456304f, 0.097353f, 0.033467f, -0.010367f, -0.00362604f, -0.00940526f, 0.0541993f, -0.0126803f, -0.0284043f, -0.126488f, 0.0276941f, -0.0072592f, -0.0112239f, 0.200614f, -0.0674165f, 0.0152713f, -0.0543701f, -0.0742834f, -0.0453187f, -0.0254072f, -0.0692672f, 0.0332971f, -0.0228297f, -0.000965714f, 0.0732683f, 0.0640799f, 0.00158938f, 0.047803f, -0.00266977f, -0.0100275f, -0.00643167f, -0.0383495f, -0.00409583f, 0.0385844f, 0.0659188f,
        0.0063133f, -0.00408226f, 0.121465f, 0.0301708f, -0.0181853f, 0.0601681f, 0.00325393f, 0.10642f, -0.0275263f, -0.0194839f, -0.0252979f, 0.0217105f, 0.0386137f, 0.0112424f, 0.0430641f, 0.0730034f, 0.0354242f, 0.013652f, -0.0293887f, 0.142649f, -0.0690173f, -0.0961422f, 0.0442838f, 0.0452969f, 0.118274f, 0.0323701f, 0.0187156f, 0.5255f, 0.0118736f, 0.225357f, -0.0130602f, -0.0104742f, -0.07411f, -0.114514f, -0.0436895f, 0.00986579f, -0.0838205f, -0.101698f, -0.00483559f, -0.00391671f, -0.0699783f, -0.0195803f, 0.0459022f, -0.0091508f, 0.0073998f, -0.0577818f, 0.0674949f, 0.0137614f, 0.0715333f, 0.00271481f, -0.00891188f, -0.0212177f, 0.0437716f, 0.0257086f, 0.0345469f, -0.180349f, -0.0603965f, -0.147289f, -0.00330522f, 0.0067096f, -0.0179399f, 0.0182082f, -0.0270762f, 0.0402878f, -0.0166916f, -0.0948335f, 0.029574f, 0.0969981f, 0.0529901f, 0.00293059f, -0.154666f, 0.0407095f, 0.0316545f, -0.0062415f, -0.0351574f, -0.0147547f, -0.0135113f, 0.00357694f, 0.0517612f, -0.101499f, -0.00291564f, -0.0056001f, -0.00857672f, -0.0101505f, -0.0323477f, -0.0263152f, -0.0116552f, 0.0247082f, 0.0227123f, -0.10951f, -0.0328793f, 0.411161f, -0.0130315f, -0.0227835f, 0.0106074f, -0.00307627f, 0.00495261f, 0.0545998f, 0.000595861f, -0.0242671f, 0.0299187f, 0.00166324f, -0.00666328f, -0.0078437f, 0.0280452f, -0.16448f, -0.0143541f, 0.026909f, -0.193269f, -0.0355148f, 0.0118665f, -0.0365043f, -0.00810059f, -0.0352678f, -0.0630561f, 0.0280126f, 0.30164f, 0.0875995f, 0.0694396f, 0.0103573f, -0.0283321f, -0.621525f, -0.0445668f, -0.0148087f, -0.313831f, -0.00408616f, 0.0349075f, 0.0231337f, 0.142115f, 0.00382164f, 0.0393434f, -0.108881f, -0.0101964f, -0.0303501f, -0.106503f, 0.0308691f, -0.0197364f, 0.0091609f, 0.00739707f, -0.021932f, 0.00100097f, 0.00910001f, -0.0272304f, 0.0244325f, -0.0534487f, -0.0124806f, 0.102616f, -0.0300018f, -0.0371498f, -0.0484335f, -0.0434477f, -0.0806446f, -0.0323094f, 0.0210301f, 0.016248f, 0.0884761f, 0.0521384f, -0.306267f, -0.0181587f, 0.0638134f, 0.00266205f, 0.0659853f, 0.0215718f, 0.030898f, -0.010891f, 0.0265176f, -0.0440084f, 0.0334551f, -0.0404191f, -0.05042f, 0.0401076f, 0.00569889f, 0.0642698f, 0.0118167f, -0.152626f, -0.0383063f, -0.241934f, -0.14967f, 0.000835922f, -0.0176463f, 0.00669299f, -0.100216f, 0.0636827f, -0.0246564f, 0.0233452f, 0.00916313f, -0.0360494f, -0.0143271f, 0.00748104f, 0.00808922f, 0.120031f, -0.0139543f, -0.0895863f, -0.0414794f, 0.143243f, -0.0137803f, 0.0207675f, -0.0347851f, 0.0721874f, -0.0414808f, -0.116213f, 0.00107106f, 0.0103554f, -0.13586f, -0.290486f, 0.00166402f, -0.015201f, -0.00145561f, -0.0154914f, 0.00163743f, 0.0822632f, 0.08017f, 0.0710966f, -0.013158f, -0.0632138f, -0.0111834f, -0.0178201f, 0.0112061f, -0.00430423f, -0.0674515f, 0.214633f, -0.00585192f, -0.0351569f, 0.375032f, 0.0448701f, 0.0256456f, 0.0743934f, 0.0211866f, -0.00896532f, -0.0415844f, 0.0122347f, 0.0118991f, -0.0877453f, 0.0304085f, -0.00665392f, -0.00567859f, -0.00832385f, 0.00138205f, 0.0402719f, -0.00329125f, -0.0122391f, 0.0130672f, -0.0699987f, -0.0336706f, 0.0130345f, -0.256598f, -0.00998923f, -0.0732391f, 0.16722f, -0.0470782f, 0.016357f, 0.0118742f, -0.0706653f, 0.00409f, -0.0124226f, 0.000505835f, -0.0507414f, 0.00258108f, 0.0198879f, 0.000320695f, 0.0112645f, 0.00723067f, -0.0107117f, -0.00964231f, 0.014985f, -0.000720747f, -0.00563631f, -0.128197f, -0.00191921f, 0.100766f, -0.0177464f, 0.0910596f, 0.132686f, 0.0851709f, 0.0140803f, -0.0459295f, 0.00891749f, 0.0917738f, -0.0520881f, -0.00429575f, -0.0104893f, -0.0285219f, 0.0370703f, -0.0241567f, 0.0214466f, 0.0260263f, 0.112436f, -0.0221967f, 0.003362f, 0.00552892f, -0.0382231f, 0.00763609f, 0.0270099f, -0.028698f, -0.00121651f, 0.000527033f, -0.0406943f, -0.0840261f, -0.00983556f, -0.0288269f, 0.00269151f, -0.136611f, 0.0220631f, -0.00476321f, 0.0281217f, 0.0243983f, -0.00436437f, 0.00491977f, 0.0540143f, 0.0410553f, -0.00945594f, -0.0711867f, -0.011407f, -0.0290617f, 0.0077444f, -0.0194761f, -0.0353022f, 0.0242323f, 0.121606f, 0.136937f, 0.117977f, 0.0648052f, 0.000369128f, -0.0286182f, -0.000851573f, -0.0675435f, 0.0374786f, 0.0108061f, -0.00134871f, -0.0419874f, 0.0271549f, -0.21822f, 0.268321f, -0.00535237f, 0.011111f, -0.0614932f, 0.0500974f, 0.0900748f, 0.0334851f, -0.101783f, -0.00498551f, -0.0075128f, 0.00031712f, 0.0485839f, 0.000919265f, 0.0326066f, -0.023036f, 0.0096988f, 0.0178391f, 0.0861196f, 0.0466213f, -0.0299909f, -0.0991148f, -0.0230341f, 0.334094f, -0.0382573f, 0.0395579f, -0.00590484f, 0.0206429f, 0.246985f, -0.0283786f, 0.0598143f, -0.0353774f, 0.091151f, 0.0944889f, 0.00249664f, 0.202462f, -0.00569812f, 0.00865333f, -0.00812537f, -0.188173f, -0.0627191f, -0.28001f, 0.00917071f, 0.0506412f, 0.0010405f, 0.0678395f, 0.16542f, -0.00219039f, 0.0110519f, -0.00379539f, 0.00535911f, -0.00791708f, -0.000717427f, -0.0325235f, 0.0842137f, -0.020968f, 0.192455f, 0.0856024f, 0.132173f, -0.00232728f, 0.0647325f, 0.104932f, -0.0235684f, 0.00335134f, 0.00515333f, 0.192284f, 0.0592319f, 0.143246f, -0.00214825f, -0.168829f, -0.0149753f, 0.00881463f, 0.00489184f, 0.0030815f, -0.0645487f, -0.236596f, 0.0211161f, 0.428909f, -0.0184283f, 0.150971f, -0.00403509f, 0.0892136f, 0.0527521f, -0.00892411f, 0.257531f, 0.0159127f, -0.0153799f, 0.0299046f, 0.00748111f, 0.02268f, -0.0283898f, -0.0224564f, -0.00329609f, -0.0642335f, 0.0385503f, 0.00387719f, -0.0795388f, 0.0385978f, 0.0338672f, -0.00181007f, 0.500546f, 0.0174027f, -0.00941603f, 0.00119533f, 0.161396f, 0.0277067f, -0.0113644f, 0.00243689f, 0.0240222f, 0.00074696f, -0.00329644f, 0.00571551f, 0.353842f, -0.0345694f, 0.0954816f, 0.022245f, 0.0639779f, -0.0209006f, -0.0100804f, -0.0223871f, 0.00248849f, -0.0231191f, -0.105286f, -0.0150994f, 0.00230265f, -0.0295301f, 0.0119341f, 0.00911531f, 0.0540066f, 0.0076047f, -0.0945892f, 0.0196067f, -0.0357786f, 0.0719775f, -0.0972845f, 0.142406f, -0.18177f, 0.00491428f, 0.000342362f, -0.0186926f, 0.0489506f, -0.0333847f, -0.017827f, -0.00585373f, 0.0250148f, -0.0496847f, 0.00595432f, 0.180951f, -0.0459607f, -0.0360709f, -0.168328f, -0.0724864f, -0.161582f, 0.0156965f, -0.0463856f, 0.00603378f, -0.0396591f, 0.100121f, 0.00849666f, 0.0438226f, 0.0247446f, 0.0309354f, -0.0876779f, -0.0223912f, 0.0149475f, -0.0619022f, -0.0198987f, 0.0258675f, 0.0760512f,
        0.0237833f, 0.00298876f, 0.0487694f, 0.00950606f, -0.074622f, 0.0192038f, -0.0202395f, 0.105125f, -0.0154085f, 0.0355691f, 0.00281225f, 0.00531638f, 0.0101454f, 0.0510713f, 0.0313131f, -3.24692e-05f, 0.0563302f, -0.00384794f, -0.0967057f, -0.00911184f, -0.034748f, -0.00885298f, -0.00145702f, 0.00841001f, -0.00386897f, 0.00954715f, 0.0060942f, -0.00779779f, 0.0341911f, 0.0373562f, 0.000677265f, -0.0620633f, 0.00208294f, -0.0215586f, -0.085074f, 0.0143441f, -0.0186877f, 0.00127867f, -0.01249f, -0.00504883f, -0.00104019f, 0.0121985f, 0.000512828f, -0.00772995f, 0.00468516f, -0.0139477f, -0.0211804f, 0.210879f, 0.00785329f, -0.000516933f, -0.00212956f, -0.0162727f, 0.00414868f, 0.0109553f, 0.000250999f, -0.00637749f, -0.00108913f, -0.00648906f, -0.0123977f, 0.0104616f, 0.0241319f, 0.0770632f, 0.00195405f, -0.00752428f, -0.0405081f, -0.0883033f, 0.0394711f, 0.0062544f, 0.0315002f, -0.0138193f, -0.0353362f, 0.00803457f, 0.0055575f, -0.00122304f, -0.00591179f, -0.000313378f, -0.00928775f, 0.00167335f, 0.00110711f, 0.0102733f, -0.0102128f, -0.0332447f, -0.0050578f, -0.0365285f, 0.00129188f, -0.00545454f, -0.0488076f, -0.0522689f, -0.0028496f, 0.0269232f, -0.00264586f, 0.00549725f, 0.0937312f, -0.0097157f, 0.000703438f, -0.0316939f, 0.00265145f, 0.00747435f, 0.00703635f, -0.0498706f, 0.0260258f, 0.00486406f, 0.00831138f, 0.00331964f, -0.0116462f, -0.000328743f, -0.0193854f, 0.012874f, -0.0140591f, 0.00294906f, 0.167637f, -0.00563081f, 0.00047881f, -0.0132155f, -0.088562f, -0.00763682f, 0.00861545f, 0.0484862f, 0.118604f, 0.00888342f, -0.0480975f, -0.0108402f, -0.00768345f, -0.214419f, -0.045855f, 0.000607434f, 0.00143275f, 0.000233664f, 0.00111974f, 0.0283561f, -0.0137152f, 0.035663f, -0.0231469f, 0.0205628f, 0.0685008f, 0.0106492f, 0.00590557f, -0.00685771f, 0.00424108f, 0.000113577f, 0.00595773f, 0.00665598f, 0.000441705f, -0.00402036f, -0.0262544f, 0.00611645f, 0.0116063f, -0.00424871f, 0.0342696f, 0.0381022f, -0.0588067f, -9.04306e-05f, 0.013434f, 0.0049054f, 0.0123942f, -0.000403249f, 0.0504587f, -0.00181204f, 0.00841684f, 0.0187689f, 0.0174106f, 0.00611652f, 0.00976013f, 0.000955711f, 0.00209072f, -0.0257193f, -0.0127599f, 0.00699173f, -0.0153516f, -0.00193625f, 0.0528177f, 0.0170662f, 0.0746572f, 0.00809554f, -0.027025f, -0.0257472f, -0.00256271f, -0.0890082f, -0.00221022f, -0.00891542f, -0.00903598f, -0.0144857f, 0.0554675f, -0.00986486f, 0.00189685f, 5.93501e-05f, 0.00462237f, 0.00532594f, 0.00433364f, -0.003124f, 0.04f, -0.000328486f, -0.0648411f, -0.00377033f, 0.139774f, 0.00230164f, 0.0115385f, 0.0125043f, 0.148022f, -0.0284796f, -0.00155402f, -0.00387695f, 0.00829478f, -0.0471497f, -0.0015643f, -0.00582674f, -0.00431319f, 0.000878919f, 0.00687072f, -0.00301133f, 0.00398096f, -0.00563914f, -0.0026393f, -0.00377055f, -0.0609272f, -0.118688f, 0.00517703f, 0.0836725f, -0.012182f, -0.0512972f, 0.0119928f, 0.0247734f, -0.0427426f, 0.0341825f, 0.0698612f, 0.00279914f, -0.00847926f, -0.0226391f, 0.020679f, -0.00144619f, -0.0104832f, 0.0195441f, 0.000150691f, 0.0815801f, -0.00616593f, 0.00379428f, -0.00447982f, 0.00261409f, 0.0600844f, -0.0213836f, -0.00804557f, 0.00325642f, 0.00854879f, -0.0814344f, -0.027769f, -0.00191851f, 0.00536533f, -0.0164033f, -0.00257131f, -0.00205376f, -0.0200541f, -0.0128954f, -0.00532982f, 0.0022407f, -0.00130887f, 0.00425618f, -0.00845818f, -0.00126148f, -0.0107566f, 0.00104842f, -0.00435674f, 0.00433842f, -0.0109865f, 0.000301519f, 0.00589863f, -0.00851759f, -0.00137109f, -0.0256632f, 0.0120122f, -0.00451766f, -0.0132172f, 0.0204377f, 0.00862719f, -0.00529603f, 0.0007616f, -0.00779072f, 0.000307369f, 0.0161384f, 0.0140168f, -0.00223271f, -0.0234216f, 0.00152691f, 0.00407567f, -0.00575267f, -0.0169706f, 0.00373715f, -0.0130443f, 0.0149063f, -0.00592504f, -0.00101738f, -0.00432452f, 0.00608682f, -0.00623923f, -0.0048846f, 0.00141049f, -0.00787022f, -0.00325903f, -0.00925192f, 4.10188e-05f, -0.00650579f, -0.00344007f, -0.00507379f, -0.010943f, 0.0033921f, 0.0262149f, -0.0109309f, -0.00218072f, 0.00487267f, -0.00424018f, 0.0190863f, -0.0205672f, -0.00521787f, -0.749656f, 0.0045255f, -0.0111087f, -0.00594957f, -0.00784532f, -0.00218566f, -0.00261733f, 0.00115839f, 0.00810127f, -0.00685174f, -0.000515265f, 0.00996413f, 0.00908507f, -0.010911f, 0.0199673f, 0.00424915f, -0.0168506f, -0.0127626f, -0.0068238f, 0.0141051f, -0.0106615f, 0.00332799f, 0.00636155f, -0.0260333f, 0.00595097f, 0.0191085f, -0.0049198f, 0.00793315f, -0.00309666f, 0.0137166f, -0.00473366f, 0.0127659f, 0.000838826f, 0.0352708f, -0.00566433f, 0.00439918f, 0.00403144f, -0.0103773f, 0.000578005f, -0.00181792f, -0.0300049f, -0.00661571f, 0.0085107f, 0.00894339f, 0.00861617f, 0.00351911f, 0.016009f, -0.00165849f, 0.00140448f, 0.00854556f, -0.000467159f, 0.00526625f, 0.0113457f, -0.000892589f, -0.00943319f, 0.016298f, 0.0129145f, 0.00977724f, -0.00864554f, -0.0149309f, 0.0109739f, 0.00925517f, 0.00301191f, -0.00253138f, -0.0198261f, 0.00383641f, 0.00511284f, -0.0561408f, -0.0281949f, -0.00444545f, -0.00338158f, -0.00161292f, -0.00978353f, 0.00446439f, 0.000485823f, 0.000591379f, 0.00729576f, -0.024535f, 0.00937071f, 0.00193014f, 0.00812366f, -0.015649f, -0.00101637f, 0.0112705f, 0.00182169f, -0.00906464f, 0.0080621f, -0.0130414f, -0.000293886f, -0.00548405f, -0.00557287f, -0.00444211f, 0.000131822f, -0.0116247f, 0.00918694f, 0.00706824f, -0.00459982f, -0.00134241f, 0.00769962f, -0.000905408f, -0.00643464f, 0.00195699f, 0.0103661f, 0.0117231f, 0.00141366f, 0.013737f, -0.00475491f, -0.00389627f, -0.008428f, -0.00336822f, -0.0123985f, -0.00384732f, -0.00772105f, -0.00399041f, 0.00441658f, -0.0179348f, 0.00088589f, 0.00130237f, -0.00910743f, -0.000932973f, -0.000705488f, -0.00845157f, -0.00409019f, -0.00198943f, -0.00037801f, -0.0110968f, -0.00639611f, 0.00967489f, -0.00286205f, -0.00142743f, 0.00952024f, 0.0067011f, -0.00771389f, 0.000101275f, 0.00173372f, 0.000959312f, 0.00841471f, 0.00336334f, 0.00371336f, 0.00482025f, -0.00711383f, 0.00583148f, 0.0108545f, -0.000470039f, -0.0110626f, 0.00324574f, 0.025979f, 0.0153801f, -0.00239289f, -0.0364105f, -0.0252222f, 0.00766028f, -0.000371992f, -0.00263989f, 0.0215774f, 0.0230998f, -0.00223724f, -0.000281751f, -0.00482297f, -0.0175295f, -0.00712851f, 0.0106509f, 0.00430235f, 0.00410187f, 0.00823292f, 0.00280169f, 8.28998e-05f, -0.00169138f, -0.00976853f, -0.00530213f, -0.00814388f, 0.0013187f, 0.00816157f, 0.00138731f, -2.68979e-05f, -0.0103893f, -0.0500543f, 0.000847671f, 0.00327953f, 0.00418289f, 0.0180997f, -0.00027566f, -0.00544788f, -0.0076323f, -0.00551657f, -0.00599236f, -0.0127374f, -0.0174632f,
        -0.000449777f, -0.000137405f, -0.0762075f, 0.000949166f, 0.0346124f, -0.0111424f, 0.0108357f, 0.0121679f, 0.0242749f, 0.052692f, -0.0017713f, 0.0053728f, 0.0128862f, -0.0162366f, 0.0125041f, -0.00602398f, 0.0107778f, -0.00323086f, -0.00914208f, -0.013884f, 0.00755173f, -0.0175622f, 0.00473339f, -0.015003f, -0.0238219f, 0.004502f, 0.00187154f, 0.0041163f, -9.36184e-05f, 0.00873372f, 0.0121869f, -0.020973f, -0.006006f, -0.0038208f, 0.00210471f, 0.00255549f, -0.0251856f, -0.0626372f, -0.0059258f, -0.0058662f, -0.0946306f, 0.00197436f, 0.00105865f, -0.0033595f, 0.0158977f, -0.0036025f, -0.00568902f, -0.0202577f, -0.000251319f, -0.0117895f, -0.0144239f, -0.0144024f, -0.0150431f, -0.0354826f, -0.0135123f, -0.000422157f, 0.0286438f, -0.000884989f, -0.00675718f, 0.013241f, -0.0118388f, 0.0321394f, -0.000803071f, 0.11408f, -0.00806301f, -0.00831608f, 0.0165189f, 0.016094f, -0.000449332f, -0.00695901f, 0.0437514f, -0.00172117f, 0.00180391f, -0.000859933f, -0.0144826f, 0.0262613f, -0.00194352f, -1.98829e-05f, -0.00902827f, -0.00400867f, -0.00600827f, 0.0120846f, -0.0162493f, 0.0418596f, 0.00131911f, -0.00631566f, 0.00270484f, -0.0950513f, 0.00726431f, -0.0169798f, -0.000554365f, -0.00256903f, -0.00885843f, 0.0104025f, 0.00590779f, -0.00175832f, 0.0168603f, 0.00964353f, -0.0180614f, 0.0213157f, 0.0209548f, -0.0231143f, -0.00121617f, -0.0129815f, -0.0199287f, 0.00863336f, -0.00464991f, 0.0162288f, -0.340115f, -0.011018f, -0.0593997f, 0.00644821f, 0.0416332f, 0.0394596f, 0.0172296f, 0.00494231f, 0.0143805f, -0.00819845f, 0.00196982f, 0.00393258f, 0.0246168f, -0.0235927f, 0.0131416f, -0.0190432f, -0.0237865f, -0.0155627f, 0.0265165f, 0.0162884f, 0.00321098f, 0.0136674f, -0.000966112f, -0.0100813f, -0.00604589f, 0.00889466f, 0.0113945f, 0.0264707f, 0.00371883f, -0.00843358f, 0.0145675f, 0.0048638f, 0.00110399f, -0.00130233f, 0.00740726f, -0.00393368f, -0.0242178f, 0.00341681f, 0.00115369f, -0.00297881f, -0.0844071f, 0.0537151f, -0.00209399f, 0.0310295f, 0.0383914f, 0.00456459f, 0.0188114f, -0.0177144f, 0.0133258f, 0.0584683f, -0.00640495f, 0.0175946f, 0.0186782f, 0.00213311f, 0.00393403f, 0.00382759f, 0.00267507f, 0.00493673f, -0.00856695f, -0.00627955f, -0.0103436f, -0.000671664f, -0.110419f, 0.0307264f, 0.0042176f, 0.0031638f, 0.0154172f, 0.00265482f, 0.0410853f, 0.00833895f, -0.0183989f, -0.000717906f, -0.0090387f, -0.00404523f, -0.00976238f, -0.0137555f, 0.000157289f, -0.00341186f, -0.0214878f, 0.0142639f, 0.00624623f, 0.000537292f, -0.0520912f, -0.0432221f, -0.00330415f, 0.0263942f, -0.00150974f, 0.00172088f, -0.0815726f, -0.0201155f, -0.00986346f, 0.0121252f, 0.00198959f, -0.0349936f, -0.00608366f, -0.00399543f, 0.0192487f, -0.0123156f, 0.0072797f, 0.000507143f, 0.0334805f, 0.000609379f, 0.00961966f, -0.00697663f, 0.00201967f, -0.0207349f, -0.0103385f, -0.00343849f, -0.00330492f, 0.035106f, -0.00456996f, 0.00197528f, 0.016148f, 0.0142903f, 0.0616483f, 0.0093118f, -0.0596028f, 0.00945764f, -0.00659242f, 0.118389f, -0.00259384f, -0.00285344f, 0.00567036f, 0.0195813f, -0.00461807f, -0.0608699f, 0.00380259f, 0.00143385f, -0.00466997f, 0.0194046f, -0.0198423f, -0.00334569f, -0.014399f, 0.0130021f, -0.0141619f, -0.00859914f, 0.00997122f, -0.0198446f, -0.0094162f, -0.0116609f, -0.0111888f, -0.00903524f, 0.00937981f, 0.01772f, -0.00236374f, -0.00870162f, 0.000141193f, -0.0343695f, -0.00997931f, 0.0073531f, -0.100394f, -0.00367661f, -0.00124499f, 0.00318026f, 0.0554203f, -0.00342582f, -0.0104147f, -0.0577869f, -0.0126485f, -0.0332496f, 0.0346141f, 0.0307962f, -0.0174745f, -0.0387339f, 0.0167707f, -0.0363424f, 0.0154902f, -0.0118644f, -4.63543e-06f, -0.0683506f, -0.0344076f, -0.00104884f, -0.00883997f, -0.00305185f, -0.0150299f, -0.0186403f, 0.0110238f, 0.00779224f, -0.0102231f, 0.0087488f, -0.0138988f, -0.0229105f, -0.0244903f, -0.0202919f, 0.00135903f, -0.00574432f, 0.00254918f, 0.0340209f, -0.046428f, -0.00670622f, 0.000925543f, -0.0249251f, -0.00275456f, 0.0199177f, 0.000210993f, 0.027762f, -0.0228046f, 0.0484813f, 0.00538959f, 0.0136714f, -0.00690097f, -0.0448533f, -0.00815204f, 0.00734891f, 0.0173959f, -0.0379109f, 0.0594617f, -0.00722084f, 0.0415935f, 0.014792f, -0.0170252f, -0.0139396f, 0.00146415f, 0.00117702f, 0.0685559f, 0.00727832f, -0.107566f, -0.0112505f, 0.033853f, 0.0046957f, -0.0242369f, 0.0148181f, -0.0723487f, -0.00961667f, 0.0304085f, -0.00520772f, -0.0316467f, 0.0327801f, -0.00755137f, 0.0166041f, -0.0557288f, -0.0227759f, -0.00314548f, 0.0152585f, 0.020071f, -0.0377076f, 0.00687613f, -0.0273935f, -0.00647955f, 0.0105047f, -0.0137238f, 0.023264f, -0.0455722f, -0.00221414f, -0.0258535f, -0.0236395f, 0.0593407f, 0.00448763f, 0.0150777f, 0.00437925f, 0.0295782f, -0.0344752f, 0.00365267f, 0.140464f, -0.0479012f, 0.025726f, 0.119063f, 0.000301925f, -0.00810565f, -0.354073f, -0.0723185f, -0.0046123f, 0.033882f, -0.044552f, -0.0138361f, 0.00384129f, 0.0139111f, -0.01667f, -0.0821503f, 0.0029974f, -0.0306725f, 0.0160366f, 0.0334754f, 0.0192693f, -0.00616713f, -0.00232275f, 0.0107987f, 0.00437057f, 0.0017298f, 0.0196916f, -0.0417255f, -0.00911193f, 0.00876709f, -0.00172422f, -0.00105248f, -0.0191631f, -0.00387423f, -0.0102766f, -0.025317f, -0.0416204f, -0.0319611f, -0.00359193f, 0.00424064f, -0.00575092f, -0.0282402f, 0.0745899f, -0.0126492f, -0.0162564f, -0.261967f, -0.705265f, -0.0403731f, -0.00209634f, -0.694297f, 0.00956909f, 0.0158826f, 0.0130207f, 0.003825f, -0.000300812f, -0.0121346f, 0.00642053f, -0.012902f, 0.0309272f, 0.0609192f, -0.00654145f, -0.0937578f, -0.00432024f, -0.00767539f, 0.0461248f, 0.00701077f, -0.0174477f, 0.00563833f, -0.0107107f, -0.0255275f, 0.00892488f, -0.00166062f, 0.039829f, -0.00150394f, 0.00742194f, -0.00885529f, -0.0103532f, 0.0777858f, 0.0885367f, -0.00425715f, 0.0423651f, -0.0446651f, -0.635069f, -0.00919329f, -0.00356176f, 0.00988705f, 0.0116529f, -0.0401253f, 0.00260105f, 0.00573955f, -0.0667439f, 0.101175f, 0.0765288f, -0.0120077f, 0.00322599f, -0.0192768f, 0.0382749f, -0.222119f, -0.0452036f, 0.0424303f, 0.0890699f, 0.0117557f, 0.0315167f, 0.0284256f, 0.00541845f, -0.250147f, 0.00420668f, -0.0189724f, -0.00416381f, -0.00162803f, -0.0108763f, -0.00970892f, 0.0134476f, -0.0254931f, 0.0307225f, 0.00128596f, 0.0171106f, 0.00467854f, -0.0124376f, 0.0183396f, 0.0021754f, 0.00170886f, -0.0226898f, 0.0250111f, -0.0533301f, -0.0163268f, 0.00618995f, 0.0416378f, 0.0475397f, 0.0105684f, -0.00440933f, 0.0496722f, -0.0215733f, -0.0256361f, -0.0285091f, -0.0276881f, -0.00102202f, -0.0720219f, -0.0296656f,
        0.00465617f, 0.00138814f, -0.0913312f, -0.0161213f, 0.0160887f, 0.0204469f, -0.0223319f, 0.015304f, 0.000397867f, 0.00824013f, 0.0114613f, 0.00408309f, 0.0384456f, -0.00453968f, 0.0176576f, 0.100434f, -0.0393971f, 0.0160015f, -0.00313166f, -0.0058054f, 0.0342083f, 0.0333727f, 0.00275399f, -0.0111393f, -0.0656798f, 0.0117794f, 0.00399766f, 0.00310487f, 0.00290905f, 0.00311256f, 0.0103328f, 0.00221549f, -0.00340486f, -0.00955604f, -0.010614f, 0.0144013f, -0.0244803f, 0.246714f, 0.00585756f, -0.0183366f, 0.0131221f, -0.015529f, 0.0634503f, -0.00107566f, 0.0230663f, -0.00523926f, -0.0100486f, -0.0270644f, 0.0938544f, -0.0136558f, 0.0164469f, -0.349288f, 0.0108305f, 0.0621752f, -0.00813808f, -0.0218271f, 0.0168811f, -0.00509217f, -0.0249135f, 0.0268669f, -0.0294336f, 0.0396944f, -0.00419361f, 0.00843219f, -0.000475472f, -0.0122415f, 0.0142385f, 0.0240099f, -0.0041296f, 0.0167314f, -0.0210217f, -0.00275032f, 0.0121842f, -0.00556776f, -0.0215306f, 0.0411878f, -0.00102203f, 0.00011487f, -0.0142263f, -0.00257424f, -0.0044306f, 0.0115836f, -0.0331884f, 0.0153153f, 0.0023461f, -0.0229996f, -0.00982945f, 0.0207273f, 0.0039542f, -0.0275622f, -0.00118208f, -0.00703868f, -0.0111554f, 0.0155981f, -0.0197133f, -0.00157645f, 0.0790344f, 0.0277319f, -0.0239723f, 0.0133704f, 0.0153687f, -0.0220235f, -0.0652554f, 0.0340702f, -0.0256995f, 0.00463251f, -0.134567f, 0.0048301f, -0.0935251f, -0.0125128f, -0.0560035f, -0.000903825f, 0.0231884f, 0.0678238f, 0.0172834f, 0.0226948f, -0.00784814f, -0.000168366f, 0.0165854f, 0.00979108f, -0.010978f, -0.147669f, 0.020833f, -0.0320907f, -0.339001f, -0.0307849f, -0.00796792f, 0.00704321f, -0.0258511f, 0.0302859f, -0.0174755f, -0.0208662f, -0.00800382f, -0.00772683f, 0.00787931f, 0.0244046f, 0.0635711f, -0.0490687f, 0.00843431f, -0.00969577f, -0.00403176f, -0.00225678f, -0.00425568f, 0.00423476f, -0.0522863f, 0.00901175f, 0.00701737f, 0.0203201f, 0.00764967f, -0.0128627f, -0.0154611f, -0.00973917f, 0.0172989f, 0.00679487f, -0.00897315f, -0.00337138f, -0.0103584f, -0.00507785f, -0.00390477f, 0.0472275f, 0.0060846f, 0.0151745f, 0.0472687f, 0.000490868f, 0.0196255f, 0.00541134f, -0.0206129f, -0.00112977f, -0.0197924f, -0.0553976f, -0.098063f, 0.0664134f, 0.00349375f, 0.00311233f, 0.0401445f, 0.0128354f, -0.0250036f, 0.0436594f, -0.0462325f, -0.00102946f, -0.013474f, -0.0172785f, 0.0394013f, -0.00569089f, 0.000160535f, 0.000504291f, 0.0504433f, -0.0205918f, 0.0101148f, -0.00946464f, -0.0885629f, -0.04032f, -0.012075f, 0.492342f, -0.000999111f, 0.00407901f, 0.0888248f, 0.0100317f, -0.024372f, -0.0211601f, 0.000658811f, -0.0209988f, -0.0190039f, -0.0219266f, -0.0516314f, -0.00642571f, 0.00488745f, 0.00512097f, 0.0145898f, -0.00157307f, 0.0026168f, 0.0156606f, -0.00531944f, -0.017507f, -0.0180003f, 0.00282254f, 0.0143295f, 0.0777137f, -0.00385748f, -0.00549398f, -0.0172826f, 0.0323722f, 0.185825f, 0.0121615f, 0.00399867f, -0.0541097f, 0.0386216f, 0.0595922f, 0.594257f, -0.00955271f, 0.00343269f, 0.0139925f, 0.00328999f, -0.0792421f, -0.045498f, 0.0113837f, -0.00976291f, 0.00624078f, -0.0254107f, -0.0216194f, -0.028773f, 0.0236943f, 0.0197444f, -0.00939094f, 0.0135671f, -0.0407697f, 0.00794318f, -0.0184558f, -0.0282076f, -0.0112124f, 0.00710705f, 0.0203747f, -0.00201855f, -0.0137849f, -0.00224183f, -0.00758043f, 0.0109492f, 0.0111736f, -0.0524165f, -0.00359813f, -0.0105491f, 0.00795013f, 0.0490089f, -0.0172285f, -0.131601f, -0.640844f, -0.00210558f, -0.0191391f, 0.144537f, -0.0187546f, -0.0117677f, -0.0243942f, -0.0673674f, 0.0116665f, -0.00634048f, -0.0171121f, -0.018849f, -0.0452217f, -0.0314511f, 0.01823f, -0.0338747f, -0.00232084f, -0.0184449f, -0.0628265f, -0.00846206f, 0.00285066f, 0.281056f, -0.0109403f, -0.036282f, 0.00725135f, -0.027479f, -0.0120889f, 0.0185699f, -0.00228023f, 0.000971992f, 0.020036f, -0.0437852f, -0.013831f, 0.0284799f, -0.0116033f, -0.0213317f, -0.0391473f, -0.0180216f, 0.0224665f, 0.00661723f, 0.0188164f, -0.00856477f, -0.0188785f, -0.0419517f, -0.0383142f, 0.00822795f, -0.0210551f, 0.0376673f, -0.0158509f, 0.0531296f, -0.0222652f, 0.0202294f, 0.0377989f, -0.0486931f, -0.0236611f, -0.0364076f, -0.0364403f, 0.105507f, -0.0520728f, -0.085646f, -0.0517868f, 0.00898522f, 0.0145328f, -0.0152412f, 0.00230019f, -0.0490983f, 0.0199105f, 0.193699f, -0.00652485f, -0.0293521f, -0.101157f, 0.00759732f, 0.0611226f, 0.00668415f, -0.0644944f, -0.00138395f, -0.0872389f, -0.0289147f, -0.0104552f, 0.0102965f, -0.00918203f, -0.0163947f, 0.00688836f, -0.0460991f, 0.0010579f, -0.0220147f, 0.00389295f, -0.0450669f, -0.0338309f, -0.00643917f, -0.164896f, 0.00520622f, -0.00943891f, 0.015696f, -0.0488516f, 0.00357405f, 0.395393f, 0.0142406f, 0.0375136f, 0.0266987f, 0.00442581f, -0.0355697f, 0.0566785f, -0.0609618f, 0.0953531f, 0.0234361f, -0.0235014f, -0.0201052f, 0.0185904f, 0.0944014f, -0.00254259f, 0.0149094f, -0.00267577f, -0.0236442f, 0.0304207f, 0.0195184f, 0.00453831f, -0.010829f, -0.00384567f, -0.00720987f, 0.00142745f, 0.00339592f, 0.0255406f, -0.0328377f, -0.0418446f, 0.00524565f, -0.019943f, -0.00744414f, -0.0262656f, -0.00295384f, -0.012041f, 0.00168772f, -0.0393009f, -0.0333347f, -0.0127033f, -0.0399219f, -0.12722f, -0.223577f, 0.0811929f, -0.130626f, -0.0705225f, 0.174048f, 0.0435034f, -0.136602f, 0.00640297f, -0.166342f, 0.0597288f, 0.0182928f, 0.00638083f, 0.00566142f, 0.0143743f, -0.0117229f, -0.00092003f, -0.00302193f, 0.0193828f, 0.0549159f, -0.01403f, -0.0686686f, -0.00131562f, -0.0395576f, 0.0140634f, 0.00728921f, -0.0222314f, 0.0847774f, 0.00397858f, -0.037106f, 0.00703206f, 0.0217107f, 0.026982f, -0.0970178f, 0.00170535f, 0.00461989f, -0.0484043f, 0.0549405f, -0.00663961f, -0.0301618f, 0.0402775f, -0.126174f, 0.042974f, 0.00767555f, -0.0323881f, -0.0021808f, 0.00152122f, -0.0794255f, 0.00950137f, 0.00617034f, -0.186531f, 0.0667047f, 0.158624f, -0.0498641f, 0.000181888f, -0.00194408f, 0.0130678f, -0.0624929f, 0.099144f, 0.00810417f, 0.174436f, 0.0147924f, 0.00815054f, 0.0152255f, -0.0833151f, -0.072767f, -0.201512f, -0.0109339f, -0.003133f, -0.00430304f, -0.0208616f, -0.0187232f, 0.0277294f, -0.451013f, 0.0336152f, -0.00462652f, 0.00806012f, -0.000483294f, 0.0313363f, 0.0948398f, -0.0302999f, -0.00779582f, -0.0975373f, 0.0429978f, -0.0117262f, -0.00451523f, -0.0175741f, 0.0914118f, 0.0390275f, 0.00306197f, 0.0172763f, 0.0486995f, -0.0628708f, -0.00845093f, 0.00565009f, -0.0126375f, 0.0362389f, -0.0893211f, -0.0264466f,
        0.0309426f, -0.0247239f, -0.0618656f, -0.16444f, 0.0416493f, -0.0039234f, -0.0446445f, -0.0806408f, 0.0315374f, -0.0123988f, 0.0385759f, 0.0315165f, 0.00742563f, -0.0276244f, 0.013597f, -0.000546713f, -0.126003f, -0.0403999f, -0.0199147f, 0.090123f, 0.0122743f, 0.0904552f, 0.0480448f, -0.0274991f, -0.0463688f, 0.132874f, -0.0163207f, 0.00931698f, 0.00050237f, -0.034227f, 0.0273549f, 0.0257694f, 0.0545361f, -0.0196519f, -0.00616926f, 0.0252382f, 0.00394299f, 0.00503618f, -0.000107687f, -0.00739968f, 0.0155088f, -0.0271828f, 0.0136159f, -0.0184294f, 0.00419291f, -0.0705982f, 0.00832841f, -0.0455188f, 0.0203078f, -0.0104058f, -0.00448528f, 0.0346675f, 0.00227903f, 0.0283768f, 0.0146701f, 0.0238016f, -0.0041065f, -0.00951874f, -0.0656203f, 0.00289312f, -0.0280637f, 0.064775f, -0.0145084f, -0.0166982f, 0.112919f, -0.030709f, -0.08767f, 0.0231176f, -0.00683745f, 0.145201f, -0.0588483f, -0.00211676f, 0.0707442f, -0.0175353f, 0.0425204f, 0.047214f, -0.00454212f, 0.108341f, -0.0655429f, -0.0661698f, -0.00742549f, 0.0525604f, -0.00200138f, 0.0760939f, 0.0208251f, -0.0183413f, -0.019956f, 0.0497461f, -0.00312012f, -0.026077f, -0.00492334f, -0.0389153f, -0.0240003f, -0.0236527f, -0.00949685f, 0.00834218f, 0.196113f, -0.0203076f, -0.0373067f, 0.0511745f, -0.000502779f, -0.0506356f, 0.0270005f, 0.0560514f, -0.0566957f, 0.00592365f, -0.0950855f, 0.0330845f, 0.0126008f, -0.0178738f, 0.00655207f, -0.00560155f, 0.0226922f, 0.122885f, -0.0227311f, -0.0185407f, -0.024025f, 0.000734875f, -0.0501656f, 0.00259467f, -0.0401208f, -0.00270448f, 0.0298842f, -0.0449168f, -0.083653f, -0.0667249f, -0.012424f, 0.0228182f, -0.0256871f, 0.0103425f, 0.00584589f, -0.0313978f, -0.00512387f, -0.0389378f, 0.00783504f, 0.0246462f, 0.0204282f, -0.0313174f, 0.0293227f, -0.0135298f, 0.0250816f, 0.00154453f, 0.00455047f, 0.0664336f, -0.0924272f, 0.0141598f, 0.0249505f, 0.0114919f, 0.127537f, -0.0302333f, -0.0464173f, 0.0312457f, 0.0119746f, 0.00862732f, -0.0221585f, -0.00284848f, 0.014157f, 0.0253277f, 0.00495452f, 0.00886403f, 0.00389645f, -0.0347684f, -0.0039163f, 0.0218669f, -0.0417104f, 0.00547612f, -0.013528f, -0.00265715f, 0.180858f, -0.000752272f, -0.18944f, 0.0260848f, -0.000632882f, 0.0126054f, 0.0359676f, 0.0302849f, -0.0371376f, 0.0941217f, -0.0281283f, -0.0280773f, -0.011986f, -0.0406752f, 0.239648f, -0.00517518f, -0.00410975f, 0.00103368f, 0.0209206f, -0.0476301f, 0.00454544f, -0.0149667f, -0.0314583f, -0.00242636f, -0.0512553f, 0.0608112f, 0.0428258f, 0.0173526f, 0.0602241f, -0.0548611f, -0.131965f, -0.0495486f, 0.00765915f, -0.062264f, -0.000979455f, -0.0652348f, -0.147691f, -0.0231597f, 0.0251012f, -0.0946399f, 0.0277068f, -0.00621829f, 0.0313192f, 0.0259072f, 0.00394534f, -0.0118648f, 0.004981f, 0.0594206f, -0.0358001f, -0.0710233f, -0.00969833f, 0.023656f, -0.0388052f, -0.00855584f, 0.259141f, 0.0142973f, -0.00158563f, 0.0164536f, 0.0212657f, 0.00174633f, 0.0514006f, -0.00881672f, 0.0221807f, 0.0413859f, 0.0143335f, -0.163744f, 0.236609f, 0.0189168f, -0.0167902f, 0.0688642f, -0.0370002f, -0.0330411f, -0.0653769f, 0.00270779f, -0.00759605f, -0.0221796f, 0.0385442f, -0.0446415f, 0.06948f, -0.033133f, -0.0352207f, -0.0310347f, 0.00721417f, 0.0857527f, 0.00283876f, -0.115239f, 0.0347347f, -0.0365242f, 0.0587821f, 0.00664576f, -0.0273541f, -0.016766f, -0.0138301f, 0.00564337f, 0.0364023f, -0.0560315f, -0.0449002f, -0.0932135f, -0.0177926f, -0.0494535f, 0.0610707f, -0.00528969f, 0.114377f, -0.0275389f, 0.0177389f, -0.0280061f, -0.00589614f, -0.00858413f, 0.0105453f, -0.0247948f, -0.0472122f, -0.000931705f, -0.0574841f, -0.0412944f, 0.00216405f, -0.0681429f, -0.00229429f, 0.00222781f, -0.0102497f, -0.0110639f, 0.0254925f, 0.0135797f, -0.0289002f, 0.00603638f, 0.0356664f, -0.0870163f, 0.552476f, 0.0106117f, -0.025193f, -0.0567232f, 0.00731144f, -0.00597211f, 0.00564131f, -0.037914f, 0.00553956f, 0.0244306f, 0.0163081f, 0.0614898f, -0.0103462f, -0.0125773f, -0.0129543f, -0.0425792f, -0.00984468f, 0.0241087f, 0.0391885f, 0.0113726f, 0.0740247f, -0.0314575f, 0.0847706f, 0.00766129f, -0.00782563f, -0.00219977f, 0.0364213f, 0.00561357f, -0.0207095f, -0.0389947f, -0.0574235f, -0.0215928f, 0.0242519f, 0.0150763f, 0.00640004f, 0.0049859f, -0.0883498f, 0.0259088f, -0.00976872f, -0.0257561f, -0.145433f, 0.0186583f, -0.0313577f, 0.0232484f, 0.135472f, -0.0611472f, -0.0134871f, -0.0152308f, 0.0481365f, -0.000509527f, 0.0241717f, -0.0205968f, -0.0464828f, 0.00742741f, -0.0585818f, 0.0174123f, -0.032865f, 0.0399474f, 0.0189778f, 0.0185407f, -0.0144228f, 0.0195944f, 0.0105867f, 0.0108527f, 0.0318328f, -0.07468f, 0.0640258f, -0.0166149f, -0.0161666f, 0.0270572f, -0.00831346f, 0.0213354f, -0.0331297f, 0.0314013f, -0.0295451f, -0.0309544f, 0.00883464f, -0.000784053f, 0.00228157f, 0.030596f, -0.0169894f, -0.0723077f, 0.0142356f, -0.042197f, -0.0273198f, 0.0607149f, 0.0824823f, 0.0722077f, -0.0207748f, -0.0090944f, 0.0268541f, 0.0273479f, 0.00481306f, -0.00487477f, -0.0183224f, -0.0126787f, 0.0311318f, -0.0985153f, -0.0152497f, 0.00489618f, -0.0141078f, -0.0060658f, -0.000568589f, -0.032613f, 0.00976906f, -0.0462634f, -0.0259696f, -0.0786609f, -0.0153404f, 0.0249492f, 0.00292531f, -0.0255124f, 0.0202219f, 0.0304817f, -0.0177191f, -0.0135411f, -0.0064023f, 0.048916f, 0.0348483f, -0.00747575f, 0.0256531f, -0.0264167f, -0.027836f, 0.026632f, -0.0408624f, 0.0405082f, 0.0435032f, -0.0481381f, 0.0232822f, 0.0406269f, -0.104934f, 0.032984f, 0.00642478f, -0.0123055f, 0.0323379f, 0.0262914f, -0.00313157f, -0.0307961f, -0.059502f, 0.043095f, -0.0842975f, -0.0634201f, -0.0069968f, -0.0269704f, 0.0525556f, -0.0145985f, -0.026517f, 0.0287775f, -0.00225143f, 0.00998218f, -0.0208695f, 0.00038333f, -0.0179813f, 0.0299511f, -0.0270286f, -0.0215702f, 0.00986492f, -0.121571f, 0.0374826f, 0.0280122f, -0.0349332f, 0.00798409f, 0.00126605f, 0.0544963f, -0.00189064f, -0.0770879f, -0.00792704f, 0.0613617f, 0.0133352f, 0.0303873f, -0.000380032f, 0.0189077f, -0.0194632f, -0.00659714f, -0.0571043f, 0.041608f, -0.0141942f, 0.012823f, 0.00537086f, 0.000970999f, 0.0332154f, 0.0570762f, -0.0137126f, 0.0101087f, -0.00108052f, -0.0265809f, -0.0247709f, -0.00362676f, -0.0148946f, 0.013131f, -0.00308769f, -0.158096f, 0.00257066f, -0.0143705f, 0.0888035f, 0.00916709f, 0.00514034f, -0.0227268f, 0.134988f, -0.0492885f, 0.0022784f, -0.0144922f, 0.0256463f, 0.0246127f, -0.0242015f, -0.0270194f,
        0.0236487f, -0.00133765f, -0.023996f, 0.0121123f, 0.0473768f, -0.0229827f, 0.0620781f, 0.0348273f, 0.0118778f, -0.0358558f, -0.00418959f, 0.026328f, 0.00159447f, -0.0285201f, 0.0242085f, 0.024281f, -0.120022f, 0.00322402f, -0.00124464f, -0.00395719f, 0.00586048f, 0.0264264f, 0.0202582f, -0.0172882f, 0.0167585f, 0.00926656f, 0.00103096f, 0.00249462f, 0.00288184f, -0.00771514f, 0.0255329f, 0.0516628f, -0.0170072f, -0.00388561f, -0.00997277f, 0.0355019f, 0.000978238f, -0.144348f, -0.00646585f, -0.013882f, 0.033804f, -0.0377087f, 0.00771159f, -0.0061665f, 0.0237085f, -0.0122598f, 0.0771705f, -0.0542605f, -0.0292168f, -0.0110855f, 0.00780249f, -0.0262439f, -0.0170252f, 0.0232333f, 0.0221474f, -0.000682905f, 0.0456239f, 0.00516233f, -0.0356498f, 0.0433573f, -0.0725911f, 0.122393f, -0.000836771f, 0.0154195f, -0.00217232f, -0.0458872f, 0.0576701f, 0.0347757f, 0.00437707f, 0.0167836f, -0.024089f, 0.00395376f, 0.0226754f, -0.000325613f, -0.0119747f, 0.0166885f, 0.0133881f, -0.00825686f, -0.0115485f, -0.0256805f, -0.013069f, 0.029991f, -0.0104672f, 0.0468771f, 0.018202f, -0.0499781f, -0.0150365f, 0.0351706f, 0.000881884f, 0.0257364f, -0.00567146f, -0.0125245f, -0.00638529f, 0.00949407f, -0.00206895f, -0.00294736f, -0.00599403f, 0.0100478f, -0.0708312f, 0.0164853f, -0.00509979f, -0.0820398f, 0.00301894f, -0.011352f, -0.103304f, 0.0361376f, -0.00276168f, 0.0140668f, 0.0182486f, -0.0224722f, 0.00670642f, -0.00173934f, -0.0763404f, 0.00545386f, -0.0451032f, 0.258199f, -0.000526159f, -0.00244376f, -0.0070213f, 0.0136966f, 0.00651444f, 0.00336226f, 0.0129456f, -0.00535145f, -0.0337439f, -0.0488545f, 0.0363396f, -0.000131419f, -0.0442874f, -0.00468587f, -0.00406768f, -0.0170205f, -0.0192772f, -0.00277597f, 0.0212662f, 0.0767458f, -0.0198272f, 0.00671115f, 0.00387314f, -0.00222632f, 0.017668f, -0.0152864f, -0.00217823f, -0.0302261f, 0.0201784f, 0.00912841f, 0.0418803f, 0.00397826f, -0.0171634f, 0.0562426f, -0.00595202f, 0.0317872f, 0.00277863f, -0.0198806f, -0.0105047f, -0.0078311f, -0.00416702f, 0.0284072f, 0.00135271f, 0.00845078f, 0.0125683f, -0.00724979f, 0.0567957f, 0.0255109f, 0.002417f, 0.0114722f, -0.0229208f, 0.00542141f, 0.000680912f, -0.0124263f, -0.0973681f, 0.0429572f, -0.00896565f, 0.00102447f, 0.0209145f, 0.0365617f, 0.00698999f, 0.0611891f, -0.0021814f, -0.00791606f, 0.0636013f, -0.0503155f, 0.041678f, -0.00722059f, -0.00547887f, 0.00243705f, -0.0177814f, -0.12321f, 0.0569086f, -0.00487058f, 0.0123446f, 0.0015868f, -0.0272469f, 0.0180903f, 0.0104843f, 0.0105209f, 0.00808024f, -0.0662313f, -0.0499085f, -0.0297908f, 0.00678693f, 0.0158422f, -0.0149847f, -0.212685f, -0.029142f, -0.0216139f, 0.0197027f, -0.00509483f, 0.0406666f, -0.00101148f, 0.0137954f, 0.0292058f, 0.0261623f, 0.0879647f, -0.0120199f, 0.0276628f, -0.00208332f, 0.00630364f, -0.00283301f, 0.0313885f, 0.00132789f, 0.00430711f, 0.131565f, 0.00856252f, -0.0451589f, 0.0151607f, -0.00609563f, 0.104563f, 0.0503204f, -0.00188153f, -0.00152094f, 0.0331939f, -0.0268272f, -0.0720271f, 0.0120254f, 0.00428272f, -0.010781f, -0.0235618f, -0.0599427f, -0.0128298f, -0.039684f, 0.0124311f, -0.00907946f, -0.0219339f, -0.00574204f, 0.00290369f, -0.0397143f, -0.0306637f, 0.0046412f, -0.102802f, 0.02052f, 0.0177221f, -0.000307451f, -0.663219f, -0.00099111f, -0.00863413f, -0.0648291f, 0.141571f, -0.0264896f, -0.00967159f, -0.0105556f, 0.00667919f, 0.019933f, -0.0081883f, -0.0256497f, -0.0425081f, -0.00260382f, -0.00437219f, 0.0181059f, 0.0588014f, -0.0156841f, -0.0992774f, 0.0577409f, -0.0112435f, 0.0118955f, -0.01259f, -1.68039e-05f, -0.0231843f, -0.0715207f, 0.00562568f, 0.00659099f, -0.00432696f, 0.0402245f, -0.0132643f, 8.8306e-05f, 0.00698941f, -0.0695019f, -0.0112349f, 0.0696259f, -0.142201f, -0.0227633f, -0.019462f, -0.0518398f, -0.0213576f, 0.0148991f, 0.0344155f, -0.0131575f, -0.012708f, -0.00177817f, -0.00639755f, -0.000887201f, -0.0257106f, -0.0247181f, 0.00548285f, 0.0290425f, 0.122557f, -0.00347772f, 0.0268244f, -0.00612725f, -0.0196236f, -0.0472946f, 0.00890478f, 0.000844572f, 0.0154442f, 0.024701f, -0.0306896f, 0.0231992f, 0.0425512f, -0.0302086f, 0.0319046f, 0.0310391f, -0.00796268f, -0.0411025f, 0.00749199f, -0.0374908f, -0.0108962f, 0.0293042f, 0.00369268f, -0.0138972f, -0.00285899f, -0.0473339f, 0.00105261f, 0.0269907f, -0.0314717f, -0.0538936f, 0.0837861f, -0.0145771f, 0.0345362f, 0.222726f, -0.034146f, -0.0154113f, 0.0519213f, 0.0351403f, -0.0609869f, 0.0181544f, -0.0165051f, 0.00702428f, -0.0109979f, -0.00444243f, -0.018915f, -0.027162f, 0.00253407f, 0.0133815f, -0.000469394f, 0.109107f, 0.0153356f, 0.00683112f, 0.0128685f, 0.0282692f, -0.0384653f, 0.000389417f, 0.106818f, 0.0799349f, 0.0567321f, 0.0479257f, 0.00394279f, -0.00575818f, -0.575371f, -0.0118667f, 0.00356253f, -0.0399865f, -0.0217626f, -0.019511f, 0.0108772f, 0.0134627f, -0.000487889f, -0.00162015f, -0.0268957f, 0.0158162f, 0.0124589f, 0.0514896f, 0.0391116f, -0.02102f, 0.0289451f, -0.0162062f, 0.0295524f, 0.0240599f, 0.00653552f, -0.0296798f, -0.0614426f, 0.00678693f, -0.0126935f, -0.0259306f, -0.0270236f, -0.005202f, -0.027559f, -0.00571665f, 0.01303f, -0.0176816f, 0.00828625f, -0.0159388f, 0.016197f, -0.0685197f, 0.0359586f, -0.0149305f, -0.0100357f, -0.054005f, 0.0405895f, -0.0436483f, -0.0196033f, 0.0205626f, 0.0601753f, 0.00745636f, 0.00526461f, 0.00770411f, -0.00536197f, -0.0196271f, -0.00742883f, 0.0673765f, 0.0225239f, 0.0330661f, -0.0197954f, 0.0635232f, -0.00196483f, -0.0160432f, 0.0274051f, 0.0249642f, -0.0215083f, 0.00376016f, 0.0484418f, -0.0339058f, -0.00930553f, 0.000391001f, 0.0489547f, 0.00680175f, 0.0121302f, -0.0159317f, -0.00746274f, 0.00762586f, 0.0151285f, -0.00984925f, 0.00967698f, -0.063813f, -0.00191317f, -0.0225768f, -0.0460198f, 0.0129389f, 0.022693f, -0.0331679f, -0.0252172f, 0.0152612f, -0.0615063f, 0.00776267f, 0.0890267f, -0.0218608f, 0.0164835f, -0.048754f, 0.0158734f, 0.00247796f, -0.0340838f, 0.0199824f, 0.0422744f, 0.00495236f, 0.00733676f, -0.693422f, -0.057195f, -0.042145f, -0.0894016f, 0.00573138f, 0.00168211f, -0.00815092f, 0.1004f, -0.00830388f, 0.0212194f, 0.00796229f, 0.0182782f, -0.00677567f, -0.0025772f, -0.0141583f, -0.0503938f, 0.00933939f, -0.0440368f, -0.0650577f, -0.0133163f, -0.0150479f, -0.128004f, -0.025883f, -0.0142512f, 0.0267747f, 0.0603829f, 0.0616747f, 0.00518816f, 0.0353825f, -0.0136665f, -0.0116953f, -0.0117363f, -0.00988685f, 0.0161024f, -0.0164802f, 0.0120735f,
        0.0115264f, 0.00956785f, -0.0348965f, -0.0115787f, 0.0441999f, 0.0345045f, 0.0134386f, -0.0337335f, -0.00245127f, -0.0610053f, 0.0043896f, 0.0019506f, 0.013525f, -0.0545739f, 0.0306072f, 0.105704f, -0.0610636f, 0.0184838f, -0.0121108f, -0.00898275f, 0.0264786f, 0.0351719f, 0.00565877f, -0.00984551f, 0.0349376f, 0.0065558f, 0.000771663f, 0.000747164f, 0.00623147f, -0.0100182f, 0.0147877f, 0.027002f, -0.0082708f, -0.00312388f, -0.031057f, 0.0352335f, 0.0102762f, -0.136548f, -0.00137814f, -0.0245331f, 0.0302073f, -0.050357f, -0.0055813f, -0.0035066f, 0.0159663f, -0.00413293f, -0.0220518f, -0.0378098f, -0.000528503f, -0.00883574f, -0.0160642f, -0.0976056f, -0.00949359f, 0.0667935f, 0.0152671f, -0.00275173f, -0.00305567f, -0.00027522f, -0.0358676f, 0.0613587f, -0.0621408f, 0.0603126f, -0.00382261f, -0.0162797f, 0.0627967f, -0.0338104f, 0.019684f, 0.0723154f, 0.0405459f, 0.0150282f, 0.0116941f, 0.0159087f, 0.0423308f, 0.000188638f, -0.0151563f, 0.0213552f, 0.0260785f, -0.000634076f, -0.00666879f, -0.0143571f, -0.0154005f, 0.0452614f, -0.0241995f, 0.00760913f, 0.00565907f, -0.0146403f, -0.00882357f, 0.109466f, 0.000185842f, 0.0530813f, -0.0167083f, -0.0132453f, 0.00510363f, 0.000928611f, -0.0231941f, -0.00849421f, -0.0127253f, 0.0143131f, -0.104331f, 0.0150856f, -0.0115339f, -0.0400927f, -0.00650179f, 0.00782663f, -0.0161432f, 0.00612369f, -0.0368485f, 0.0320765f, -0.000285285f, -0.0252538f, 0.00567933f, -0.00326235f, -0.0118118f, -0.0067807f, -0.0626707f, 0.0314245f, -0.00367115f, 0.0034559f, 0.00094028f, 0.012767f, -0.0376215f, -0.0102952f, 0.0236869f, 0.00184345f, -0.0418395f, -0.0542331f, -0.00655869f, -0.00491183f, -0.0167015f, -0.0135059f, -0.0126727f, -0.0262544f, -0.0235505f, -0.00927455f, 0.044421f, 0.0340354f, 0.0544527f, 0.0133111f, 0.00308665f, 0.00078136f, -0.0023735f, -0.0141342f, 0.00124783f, -0.0175074f, 0.0506524f, 0.0344784f, 0.016513f, 0.00434411f, -0.0224391f, 0.0865785f, -0.00372209f, -0.0103298f, -0.00164323f, -0.0143697f, -0.0125625f, -0.00602005f, -0.00435671f, -0.0097799f, -0.00277924f, 0.0124438f, 0.00866435f, 0.00456806f, 0.032294f, 0.00501145f, 0.0381001f, 0.0142146f, -0.0373586f, -0.0278584f, -0.0268059f, -0.0109542f, 0.0129881f, -0.0289077f, -0.00849425f, 0.00391238f, 0.0105073f, 0.0449334f, 0.00855353f, 0.0402285f, -0.00646413f, -0.00671409f, 0.013527f, -0.0528845f, 0.0319318f, -0.0113917f, -0.0113392f, -0.000316065f, 0.0412851f, -0.0162739f, 0.0137208f, -0.0163712f, 0.0349673f, 0.00457418f, -0.0198638f, 0.0765183f, -0.001026f, 0.0113388f, 0.00846672f, 0.0122229f, -0.0401006f, -0.00219702f, 0.00703645f, 0.0321573f, 0.000362714f, -0.24312f, -0.014646f, -0.00614563f, 0.0187569f, -0.00394876f, 0.0243838f, -0.00188284f, 0.0050112f, 0.0221267f, -0.00302741f, 0.0435336f, -0.0226377f, 0.0262879f, 0.0155468f, 0.0279725f, -0.00188527f, -0.00564561f, -0.00020769f, 0.0150204f, 0.13116f, 0.021348f, 0.00731956f, -0.0343524f, 0.00212442f, 0.0352829f, 0.526485f, -0.00325235f, -0.00250349f, 0.0161844f, -0.0453576f, -0.0154224f, -0.0407768f, 0.0031079f, -0.00879997f, 0.00831367f, -0.0461003f, -0.0249753f, -0.0173187f, 0.0510597f, 0.0221946f, -0.0149577f, 0.000957178f, 0.0111411f, 0.00876051f, -0.0220329f, -0.0046637f, -0.020372f, 0.00369127f, 0.039286f, -0.00385722f, 0.0115072f, -0.00474474f, -0.0141273f, -0.19162f, -0.0187427f, -0.00145075f, -0.00458649f, -0.00136821f, 0.0037382f, 0.0102019f, -0.0101349f, -0.0303892f, -0.697959f, -0.00391341f, -0.00169856f, 0.0454146f, -0.0300301f, -0.0387779f, -0.0249505f, -0.0183996f, -0.00471838f, -0.00533851f, 0.000305908f, -0.00737827f, -0.0143906f, -0.0612462f, 0.0117793f, -0.0296389f, -0.0045701f, 0.0974987f, -0.0222056f, -0.00917552f, 0.00540695f, 0.376f, -0.0369584f, 0.0818413f, -0.0806179f, -0.0591828f, -0.0292424f, 0.0175326f, -0.0141385f, 0.01833f, 0.0209717f, -0.0198613f, -0.0303378f, -0.00184021f, -0.095508f, 0.00121903f, 0.00795399f, -0.0660669f, -0.000692821f, 0.00370955f, 0.140168f, -0.000690335f, 0.0085036f, -0.0224978f, 0.0989872f, -0.103726f, -0.00133824f, 0.00176511f, 0.0226218f, 0.00723803f, -0.0136401f, 0.0136266f, 0.00908615f, -0.0421018f, -0.0535609f, -0.0230947f, -0.0338358f, -0.00108633f, -0.0356084f, -0.109221f, -0.014515f, 0.0077523f, 0.0139792f, -0.0248496f, -0.023008f, -0.0472426f, 0.0865438f, 0.000595621f, -0.0451802f, -0.0395005f, 0.0493621f, -0.00124904f, 0.0988936f, 0.0572095f, -0.0729679f, -0.00415711f, 0.161504f, -0.00328739f, -0.0133308f, 0.00799106f, -0.0163052f, -0.0209516f, 0.00308542f, -0.0129289f, -0.0510538f, -0.0122714f, -0.0362058f, 0.0683402f, -0.0126313f, 0.0263825f, 0.0168551f, 0.00470125f, 0.0204198f, 0.0145374f, -0.021401f, 0.00460656f, 0.085484f, 0.0781075f, 0.0251125f, 0.00791536f, -0.0189591f, -0.0431845f, 0.051558f, 0.017842f, 0.36608f, -0.0343333f, -0.0303445f, -0.0115494f, 0.0530173f, 0.0165506f, -0.0235855f, -0.052452f, -0.00888096f, 0.0221193f, 0.0386185f, 0.0353902f, 0.0246971f, -0.0122489f, 0.0512722f, 0.00400143f, 0.0255521f, 0.00548785f, 0.00233302f, -0.0253462f, -0.0966852f, 0.00378993f, 0.00350757f, -0.0310213f, -0.0279353f, -0.00233223f, -0.0220107f, 0.00163079f, -0.00717164f, 0.00659987f, -0.00608499f, -0.02305f, 0.00402512f, -0.32546f, 0.0706807f, 0.0274278f, 0.0267394f, -0.00604822f, 0.0361692f, -0.0515999f, 0.0369351f, 0.0124044f, 0.0716815f, 0.0053833f, 0.00673388f, 0.0250085f, -0.000686182f, -0.00550432f, -0.00231397f, 0.00181825f, 0.022164f, 0.0330005f, -0.00140523f, 0.0463948f, -0.0278037f, -0.0318544f, 0.0275073f, 0.0620945f, -0.0128747f, 0.0329174f, 0.0206743f, -0.0352932f, -0.00835452f, 0.0248623f, 0.119621f, -0.0292978f, -0.0132096f, -0.0302576f, -0.0178306f, 0.0209123f, 0.0229405f, -0.0236861f, 0.00108116f, -0.0799521f, 0.00532662f, 0.0127616f, -0.00190055f, 0.00847102f, 0.00451121f, -0.0637118f, -0.0302129f, 0.0119081f, -0.117328f, -0.00946109f, 0.0605782f, -0.0390624f, 0.0192556f, -0.0170363f, 0.0300991f, 0.0444662f, 0.0422317f, 0.0170539f, 0.0504948f, 0.0270332f, 0.00916911f, 0.0242343f, 0.00898315f, -0.0158267f, -0.0475899f, 0.0175909f, -0.000817633f, -0.0176624f, 0.0975135f, -0.00854145f, 0.0155055f, 0.00762038f, 0.0229743f, -0.0525053f, -0.0149161f, -0.0367894f, -0.104801f, 0.013039f, -0.120883f, -0.0715135f, -0.0193206f, 0.0158965f, -0.0748989f, -0.120509f, -0.0506567f, 0.0147239f, 0.107749f, 0.0659703f, 0.0220761f, 0.0242295f, 0.0180054f, -0.0111281f, -0.0171504f, -0.014431f, 0.083154f, 0.0241038f, 0.0115941f,
        0.0112054f, -0.208447f, -0.0871743f, -0.0362684f, -0.0110118f, 0.068481f, 0.0322887f, -0.0375058f, -0.0130676f, -0.101841f, 0.0479009f, 0.0459907f, 0.00208143f, -0.0880017f, 0.0160549f, -0.0533964f, -0.0336657f, -0.000403741f, 0.0274574f, 0.00649047f, -0.0278283f, -0.0254132f, 0.0467184f, -0.0375531f, 0.127941f, 0.0291329f, 0.00155753f, 0.00199031f, 0.0183402f, 0.155697f, 0.0500429f, 0.00407514f, 0.0229933f, -0.00482785f, -0.0220735f, 0.0390895f, -0.0863406f, -0.132777f, 0.00204372f, -0.0069423f, 0.0260759f, -0.031759f, -0.00107891f, -0.0218382f, 0.00464639f, -0.00370248f, 0.00721869f, -0.0152541f, -0.00113688f, -0.00731756f, -0.0459436f, -0.0122795f, -0.0212339f, 0.072953f, 0.0268922f, -0.00254329f, -0.00535364f, 0.0200235f, -0.019393f, 0.00740422f, -0.0515143f, 0.0410708f, -0.00789718f, -0.0633389f, 0.0544137f, -0.0580859f, 0.0325159f, -0.015541f, 0.0178216f, 0.289658f, -0.0234133f, -0.0074536f, 0.0255261f, 0.00291012f, -0.0219596f, 0.0246941f, -0.00560577f, 0.00899517f, 0.00914874f, -0.0254892f, -0.0521876f, 0.0629406f, -0.00645591f, 0.111561f, 0.0122516f, -0.0106223f, -0.0132192f, -0.0819937f, 0.0132221f, -0.00695472f, -0.0207924f, -0.0723628f, 0.0495887f, -0.0359372f, -0.04756f, -0.0288064f, -0.08486f, 0.285901f, -0.0527237f, 0.0401743f, 0.00317573f, -0.00912604f, -0.00509804f, -0.019646f, -0.0133663f, 0.00250147f, 0.00489291f, 0.017901f, 0.117288f, -0.0253837f, 0.0201622f, -0.0127631f, -0.000326688f, -0.0153231f, -0.0756543f, 0.113002f, -0.0181392f, -0.00927301f, 0.0726324f, 0.00722584f, -0.0730271f, 0.0245927f, -0.102462f, 0.0356965f, -0.0606429f, -0.0444952f, -0.0166311f, 0.00795211f, -0.00189904f, -0.0158499f, -0.0204771f, -0.0472794f, -0.0079858f, -0.0501545f, 0.102751f, 0.0584957f, 0.0372233f, 0.00862791f, 0.00449617f, -0.0237138f, 0.00679621f, -0.0152089f, -0.00387291f, -0.126512f, -0.0284672f, -0.0684034f, 0.0303137f, -0.0162955f, -0.0581197f, -0.220276f, -0.00417518f, -0.0689113f, -0.017655f, -0.0224894f, 0.0357768f, 0.0133865f, 0.022937f, 0.0472434f, -0.00953042f, -0.0159915f, 0.00998823f, 0.00600883f, 0.0533401f, 0.194183f, 0.477756f, 0.0191196f, 0.0227464f, -0.00284643f, -0.13471f, 0.0769816f, 0.01241f, -0.0497929f, -0.0935632f, 0.0292851f, 0.0178327f, 0.104592f, -0.0467304f, -0.00100124f, -0.0401962f, -0.0224538f, -0.00678469f, -0.073481f, 0.227438f, -0.00830996f, 0.073789f, -0.0239749f, 0.154952f, -0.0544236f, 0.0156297f, 0.19281f, 0.0326588f, -0.00926173f, -0.0288493f, 0.0228173f, 0.0186095f, 0.0415022f, 0.0290895f, -0.00247426f, -0.0898812f, 0.0274265f, 0.0393059f, 0.0222607f, 0.019877f, -0.150684f, -0.262853f, -0.0894445f, -0.0205114f, -0.00142168f, 0.126473f, -3.85201e-05f, 0.0356633f, 0.0269576f, 0.0157574f, -0.0432543f, 0.0279592f, 0.024804f, -0.0267448f, 0.0191669f, -0.0040675f, 0.0139007f, 0.00963236f, -0.0110146f, 0.137714f, 0.0166686f, 0.0200946f, -0.0611695f, -0.0639973f, 0.0055134f, 0.042783f, 0.0271225f, -0.0468356f, 0.0247138f, 0.0103724f, 0.00932251f, -0.0140851f, 0.0358128f, -0.0059887f, 0.0386251f, -0.00545864f, 0.0596616f, -0.0379678f, 0.0116168f, -0.0113317f, -0.0299328f, 0.0217457f, 0.0063076f, -0.00526829f, -0.012835f, 0.0163333f, -0.0390477f, 0.0108823f, 0.127479f, -0.00949771f, 0.000669599f, -0.00832522f, -0.00771118f, -0.554012f, -0.259737f, 0.00827122f, -0.000538992f, 0.0152035f, 0.05717f, 0.00494831f, -0.0414577f, 0.0166355f, 0.0400496f, -0.0114314f, -0.0214246f, 0.00867137f, -0.0404191f, -0.0166356f, 0.0428265f, 0.0146152f, 0.00234592f, -0.0864799f, 0.0226774f, 0.00508847f, 0.0203778f, -0.0583453f, -0.00666855f, -0.127756f, -0.00862127f, 0.0452925f, -0.0831513f, -0.00326817f, 0.00995622f, 0.116901f, -0.0877858f, 0.112396f, -0.102312f, -0.105516f, -0.0259396f, 0.00757632f, 0.00122858f, 0.0103624f, 0.0457345f, -0.0242102f, -0.0583132f, -0.012498f, -0.313943f, 0.0069556f, -0.0319396f, 0.0172862f, -0.00853725f, 0.0116005f, 0.125311f, 0.00419865f, 0.0476964f, 0.00896339f, 0.00977134f, -0.0925261f, 0.0156905f, -0.018496f, 0.0196972f, 0.0157389f, -0.00196949f, 0.0145061f, -0.0606428f, -0.0694258f, 0.0709404f, 0.00871243f, 0.00455373f, -0.00558034f, -0.0824924f, 0.0011513f, 0.0384797f, 0.00638306f, 0.00363507f, -0.0606946f, -0.0774373f, -0.020545f, 0.0937525f, -0.00557294f, -0.0987101f, -0.0864387f, -0.0108511f, -0.0149365f, 0.0481765f, 0.036998f, -0.112909f, -0.00983293f, 0.135054f, 0.071086f, 0.019128f, 0.00687174f, -0.00651517f, -0.0349884f, -0.00583317f, -0.0110052f, 0.0398168f, -0.0141334f, -0.0344924f, -0.00134893f, -0.0270122f, -0.000114596f, 0.0220215f, -0.0321631f, 0.0329176f, 0.0261847f, 0.170964f, 0.0083325f, -0.0209986f, -0.00422142f, 0.00124639f, -0.000368193f, 0.00871341f, -0.0488562f, 0.0170233f, 0.0236273f, -0.0163899f, -0.120393f, -0.151225f, -0.00206636f, 0.105974f, -0.00312998f, 0.0290657f, -0.014112f, -0.0107348f, 0.032648f, 0.026346f, 0.0817057f, 0.0319139f, 0.00208954f, 0.0860523f, 0.0385837f, 0.115668f, 0.0399777f, 0.00339539f, -0.00545459f, 0.0598012f, 0.00298756f, 0.0148942f, -0.0227489f, -0.0139737f, -0.00473305f, -0.0326132f, -0.0229166f, 0.0207593f, 0.00277288f, -0.00719371f, -0.0202886f, -0.00475025f, 0.00617697f, 0.0162748f, 0.124789f, 0.0101917f, -0.0547861f, 0.0414249f, -0.0484098f, -0.0963767f, 0.0484124f, -0.00538394f, 0.00789277f, -0.0102249f, 0.104348f, 0.00192805f, 0.00647828f, -0.0461272f, -0.00422982f, 0.0325315f, 0.0211869f, 0.0120108f, 0.0362735f, -0.100353f, -0.106846f, 0.0453949f, 0.108593f, -0.00433587f, 0.0477131f, -0.011734f, -0.00221842f, -0.0186096f, 0.0176472f, 0.0535756f, -0.00753715f, -0.00288954f, 0.0351324f, -0.000401414f, 0.0260439f, 0.0353749f, -0.0214858f, 0.000924544f, 0.000524206f, 0.0411247f, -0.0106592f, 0.0429043f, 0.0430829f, 0.029413f, -0.00455873f, -0.0157798f, 0.0105745f, -0.10904f, -0.0209529f, -0.0605732f, 0.0151213f, 0.0293344f, -0.030329f, 0.0388919f, 0.0830521f, -0.0636824f, 0.0834155f, -0.0213396f, -0.0029129f, 0.0130443f, 0.0276463f, 0.0168069f, 0.0131674f, 0.00569299f, 0.0319241f, -0.215148f, -0.080939f, 0.033579f, -0.0317194f, 0.0329596f, -0.000564258f, 0.0486169f, -0.0763688f, -0.0114993f, -0.0945774f, -0.0518434f, 0.0208386f, -0.059539f, -0.109562f, 0.0544319f, -0.0264226f, -0.090276f, -0.0850728f, -0.0434345f, -0.00627017f, 0.0531473f, 0.214103f, -0.0206121f, 0.0996398f, 0.155764f, -0.00199676f, -0.0115997f, -0.0355156f, 0.113538f, 0.0365645f, 0.0733962f,
        -0.00286558f, -0.000197294f, -0.00342685f, 0.0153883f, -0.0289065f, -0.00108885f, 0.0387835f, 0.00578341f, -0.0262422f, 0.0582573f, -0.0156079f, 0.00150583f, -0.0362458f, 0.0104373f, 0.0113533f, -0.0386981f, -0.00206408f, 0.00844815f, -0.053396f, -0.00516133f, -0.0270117f, -0.27301f, 0.0846544f, -0.00616813f, 0.0871727f, 0.00543487f, 0.00184716f, 0.000983837f, 0.0179262f, 0.0818094f, 0.00281717f, 0.000740774f, -0.19657f, -0.00320105f, -0.00607788f, 0.0319331f, -0.0235907f, -0.0270345f, -0.00849474f, -0.0110374f, -0.0746362f, -0.00860617f, -0.0237307f, -0.00439848f, 0.0264778f, -0.00958103f, 0.019552f, -0.00263648f, 0.0296515f, -0.00113696f, -0.124069f, -0.0114847f, 0.00308178f, 0.0198766f, 0.0900992f, 0.00727182f, -0.0940811f, 0.0216224f, -0.0459814f, 0.017883f, -0.0572063f, 0.0627439f, -0.00563362f, -0.0648773f, -0.00153448f, -0.0913932f, 0.0230685f, 0.00374598f, 0.034431f, 0.112274f, -0.326763f, 0.0153011f, 0.00631464f, 0.00570424f, -0.0595568f, -0.0114755f, -0.0054627f, -0.000223818f, -0.00287856f, -0.0252934f, -0.0108832f, 0.0186247f, -0.0140906f, -0.0239924f, -0.0332168f, -0.00818134f, -0.0261136f, 0.0112761f, -0.0570478f, -0.0484226f, -0.00280576f, -0.140804f, -0.0192205f, 0.0193394f, 0.0043392f, -0.0096851f, -0.0238295f, 0.0496011f, -0.00870349f, 0.0271804f, 0.00735628f, 0.00931979f, -0.000362012f, -0.00038859f, 0.098518f, -0.0510564f, -0.00233872f, 0.00517725f, 0.0101231f, -0.0331861f, 0.0441328f, -0.00924161f, -0.0059294f, -0.0159056f, -0.0810096f, 0.203707f, 0.00935022f, 0.00920423f, 0.0427866f, 0.0270535f, -0.0613705f, 0.0281747f, -0.0292151f, -0.011845f, -0.560809f, -0.0430764f, 0.0249193f, 0.001065f, 0.0495798f, -0.00604974f, -0.0115863f, -0.0841969f, -0.0400231f, -0.0234006f, -0.099013f, 0.0434646f, 0.000907694f, 0.0125445f, 0.0118042f, -0.00467421f, 0.00886041f, 0.0296945f, 0.0324396f, -0.0114072f, 0.0988003f, 0.00847453f, 0.0464346f, -0.00464305f, -0.0289332f, 0.0590643f, -0.0350208f, -0.0899201f, -0.0159029f, -0.0648027f, 0.00696909f, -0.00101221f, 0.0140877f, -0.0855302f, -0.000846032f, -0.0256277f, 0.00854884f, -0.00292961f, 0.209544f, 0.0872828f, 0.0246488f, 0.0291603f, -0.0784974f, 0.00920441f, -0.011242f, -0.0297102f, 0.0152799f, -0.0428288f, -0.0651387f, 0.0138869f, 0.0139815f, 0.0836656f, 0.0361113f, -0.0635471f, -0.0160178f, -0.0220017f, 0.234027f, -0.0400384f, 0.186927f, -0.0295061f, 0.00130944f, -0.0287178f, -0.0214042f, -0.0285818f, 0.0222618f, -0.00368823f, 0.0601194f, -0.0188088f, -0.0146725f, 0.0157483f, 0.21603f, 0.056817f, -0.20685f, -0.0254415f, 0.00525571f, 0.00219887f, 0.0530388f, 0.0607272f, 0.0061378f, -0.113869f, -0.16334f, -0.0464161f, -0.00694523f, -0.00488537f, 0.0286918f, 0.00290496f, 0.178755f, 0.0109929f, 0.110835f, -0.0642967f, -0.0333608f, 0.00169389f, -0.00546941f, 0.00973807f, -0.00576067f, -0.0205257f, 0.0511577f, -0.0266243f, 0.109812f, 0.0471989f, 0.0996845f, 0.0135877f, -0.0794984f, 0.0649346f, 0.0303168f, -0.0011697f, 0.00521801f, 0.0626395f, -0.00297682f, 0.0266726f, -0.000223535f, 0.0116355f, -0.0108245f, 0.000611158f, 0.00728507f, 0.0239288f, -0.00188282f, 0.0150957f, -0.040548f, -0.0589448f, 0.0328252f, -0.0915972f, 0.0805046f, -0.00811939f, 0.0772469f, -0.0716012f, 0.000604462f, 0.047583f, 0.0334997f, -0.000381467f, -0.00726828f, 0.00027943f, -0.0427843f, -0.0568598f, 0.0147649f, -0.00348073f, 0.00288838f, 0.00979242f, -0.00538436f, -0.024106f, 0.00541673f, 0.00529046f, -0.00278852f, -0.0222607f, -0.00626747f, 0.0973789f, -0.0795939f, 0.105127f, -0.337742f, 0.0172115f, 0.00255328f, -0.0330435f, 0.0063678f, 0.0471297f, -0.050865f, -0.00217128f, 0.0139913f, -0.00278459f, 0.0452206f, -0.0122722f, 0.00537665f, 0.0068003f, -0.0241691f, -0.00537261f, 0.00198657f, 0.0288662f, -0.0673232f, -0.00391073f, 0.0160158f, -0.0148616f, 0.00889894f, 0.0278599f, -0.0259723f, -0.0464762f, -0.0699778f, 0.0855682f, -0.00447207f, -0.105144f, -0.000995281f, -0.0146742f, -0.49647f, 0.0685417f, -0.000740646f, 0.0278313f, -0.00761982f, 0.0475931f, -0.0645097f, 0.119236f, -0.0570179f, 0.00915969f, 0.0156965f, 0.101129f, -0.0274397f, 0.0317f, 0.435965f, 0.0895423f, 0.0228896f, 0.0537683f, -0.0312062f, -0.0316729f, 0.00405423f, -0.00417011f, 0.053186f, 0.0124111f, -0.0636419f, -0.059223f, 0.00212677f, -0.00180764f, -0.0184438f, -0.00539991f, -0.0216965f, -0.0297828f, -0.00665945f, 0.0659594f, 0.109878f, -0.0859683f, -0.0195527f, 0.0856906f, 0.113261f, 0.0901811f, 0.00573377f, 0.0357797f, -0.0261576f, 0.0127095f, 0.00452054f, 0.0160191f, 0.0674667f, -0.0187489f, 0.00896214f, -0.00895184f, 0.388793f, 0.0155203f, -0.206128f, -0.0134212f, 0.0159576f, 0.240592f, -0.0244503f, 0.0595618f, 0.0056212f, -0.0505254f, 0.160077f, 0.0021605f, 0.111341f, -0.664956f, 0.031356f, -0.00658282f, -0.431486f, -0.0241319f, -0.437714f, 0.0186697f, 0.0143805f, -0.0139802f, -0.00777148f, 0.0223012f, -0.0458929f, 0.0103136f, 0.0203269f, -0.0121667f, -0.00358236f, -0.0347832f, 0.0310102f, 0.0940264f, 0.0402878f, 0.0779475f, 0.085935f, 0.0506573f, 0.0125433f, 0.00945608f, 0.00711064f, -0.0157027f, -0.00267093f, -0.0460969f, 0.00133153f, 0.0510218f, 0.0568231f, 0.00654478f, -0.0148599f, -0.00556127f, 0.0984337f, 0.0012008f, 0.0401073f, -0.00218267f, -0.0913605f, 0.0250143f, 0.0269926f, -0.00189873f, 0.145338f, -0.0106285f, 0.128684f, 0.0182833f, -0.0104387f, 0.058272f, 0.054818f, -0.0204594f, 0.0514151f, -0.0114196f, 0.0121938f, -0.0135972f, 0.00423344f, 0.0268584f, -0.0233103f, 0.0149913f, 0.00556167f, 0.175006f, 0.0460865f, -0.0531133f, -0.00530817f, 0.00775018f, -0.00568381f, 0.00309299f, 0.00404426f, 0.0611169f, 0.04162f, 0.0620172f, 0.0113454f, 0.0556293f, -0.000326539f, -0.0136839f, -0.00373327f, 0.0962103f, -0.0169842f, 0.0247842f, 0.0442757f, 0.0244144f, -0.0176649f, -0.00554654f, -0.0050203f, -0.0177601f, -0.02368f, 0.0243078f, -0.0571087f, 0.0184628f, -0.0841841f, 0.0331607f, 0.0279732f, -0.0822138f, 0.0293232f, -0.0722001f, 0.0163439f, 0.0191851f, 0.414194f, 0.456304f, 0.097353f, 0.033467f, -0.010367f, -0.00362604f, -0.00940526f, 0.0541993f, -0.0126803f, -0.0284043f, -0.126488f, 0.0276941f, -0.0072592f, -0.0112239f, 0.200614f, -0.0674165f, 0.0152713f, -0.0543701f, -0.0742834f, -0.0453187f, -0.0254072f, -0.0692672f, 0.0332971f, -0.0228297f, -0.000965714f, 0.0732683f, 0.0640799f, 0.00158938f, 0.047803f, -0.00266977f, -0.0100275f, -0.00643167f, -0.0383495f, -0.00409583f, 0.0385844f, 0.0659188f,
        0.0063133f, -0.00408226f, 0.121465f, 0.0301708f, -0.0181853f, 0.0601681f, 0.00325393f, 0.10642f, -0.0275263f, -0.0194839f, -0.0252979f, 0.0217105f, 0.0386137f, 0.0112424f, 0.0430641f, 0.0730034f, 0.0354242f, 0.013652f, -0.0293887f, 0.142649f, -0.0690173f, -0.0961422f, 0.0442838f, 0.0452969f, 0.118274f, 0.0323701f, 0.0187156f, 0.5255f, 0.0118736f, 0.225357f, -0.0130602f, -0.0104742f, -0.07411f, -0.114514f, -0.0436895f, 0.00986579f, -0.0838205f, -0.101698f, -0.00483559f, -0.00391671f, -0.0699783f, -0.0195803f, 0.0459022f, -0.0091508f, 0.0073998f, -0.0577818f, 0.0674949f, 0.0137614f, 0.0715333f, 0.00271481f, -0.00891188f, -0.0212177f, 0.0437716f, 0.0257086f, 0.0345469f, -0.180349f, -0.0603965f, -0.147289f, -0.00330522f, 0.0067096f, -0.0179399f, 0.0182082f, -0.0270762f, 0.0402878f, -0.0166916f, -0.0948335f, 0.029574f, 0.0969981f, 0.0529901f, 0.00293059f, -0.154666f, 0.0407095f, 0.0316545f, -0.0062415f, -0.0351574f, -0.0147547f, -0.0135113f, 0.00357694f, 0.0517612f, -0.101499f, -0.00291564f, -0.0056001f, -0.00857672f, -0.0101505f, -0.0323477f, -0.0263152f, -0.0116552f, 0.0247082f, 0.0227123f, -0.10951f, -0.0328793f, 0.411161f, -0.0130315f, -0.0227835f, 0.0106074f, -0.00307627f, 0.00495261f, 0.0545998f, 0.000595861f, -0.0242671f, 0.0299187f, 0.00166324f, -0.00666328f, -0.0078437f, 0.0280452f, -0.16448f, -0.0143541f, 0.026909f, -0.193269f, -0.0355148f, 0.0118665f, -0.0365043f, -0.00810059f, -0.0352678f, -0.0630561f, 0.0280126f, 0.30164f, 0.0875995f, 0.0694396f, 0.0103573f, -0.0283321f, -0.621525f, -0.0445668f, -0.0148087f, -0.313831f, -0.00408616f, 0.0349075f, 0.0231337f, 0.142115f, 0.00382164f, 0.0393434f, -0.108881f, -0.0101964f, -0.0303501f, -0.106503f, 0.0308691f, -0.0197364f, 0.0091609f, 0.00739707f, -0.021932f, 0.00100097f, 0.00910001f, -0.0272304f, 0.0244325f, -0.0534487f, -0.0124806f, 0.102616f, -0.0300018f, -0.0371498f, -0.0484335f, -0.0434477f, -0.0806446f, -0.0323094f, 0.0210301f, 0.016248f, 0.0884761f, 0.0521384f, -0.306267f, -0.0181587f, 0.0638134f, 0.00266205f, 0.0659853f, 0.0215718f, 0.030898f, -0.010891f, 0.0265176f, -0.0440084f, 0.0334551f, -0.0404191f, -0.05042f, 0.0401076f, 0.00569889f, 0.0642698f, 0.0118167f, -0.152626f, -0.0383063f, -0.241934f, -0.14967f, 0.000835922f, -0.0176463f, 0.00669299f, -0.100216f, 0.0636827f, -0.0246564f, 0.0233452f, 0.00916313f, -0.0360494f, -0.0143271f, 0.00748104f, 0.00808922f, 0.120031f, -0.0139543f, -0.0895863f, -0.0414794f, 0.143243f, -0.0137803f, 0.0207675f, -0.0347851f, 0.0721874f, -0.0414808f, -0.116213f, 0.00107106f, 0.0103554f, -0.13586f, -0.290486f, 0.00166402f, -0.015201f, -0.00145561f, -0.0154914f, 0.00163743f, 0.0822632f, 0.08017f, 0.0710966f, -0.013158f, -0.0632138f, -0.0111834f, -0.0178201f, 0.0112061f, -0.00430423f, -0.0674515f, 0.214633f, -0.00585192f, -0.0351569f, 0.375032f, 0.0448701f, 0.0256456f, 0.0743934f, 0.0211866f, -0.00896532f, -0.0415844f, 0.0122347f, 0.0118991f, -0.0877453f, 0.0304085f, -0.00665392f, -0.00567859f, -0.00832385f, 0.00138205f, 0.0402719f, -0.00329125f, -0.0122391f, 0.0130672f, -0.0699987f, -0.0336706f, 0.0130345f, -0.256598f, -0.00998923f, -0.0732391f, 0.16722f, -0.0470782f, 0.016357f, 0.0118742f, -0.0706653f, 0.00409f, -0.0124226f, 0.000505835f, -0.0507414f, 0.00258108f, 0.0198879f, 0.000320695f, 0.0112645f, 0.00723067f, -0.0107117f, -0.00964231f, 0.014985f, -0.000720747f, -0.00563631f, -0.128197f, -0.00191921f, 0.100766f, -0.0177464f, 0.0910596f, 0.132686f, 0.0851709f, 0.0140803f, -0.0459295f, 0.00891749f, 0.0917738f, -0.0520881f, -0.00429575f, -0.0104893f, -0.0285219f, 0.0370703f, -0.0241567f, 0.0214466f, 0.0260263f, 0.112436f, -0.0221967f, 0.003362f, 0.00552892f, -0.0382231f, 0.00763609f, 0.0270099f, -0.028698f, -0.00121651f, 0.000527033f, -0.0406943f, -0.0840261f, -0.00983556f, -0.0288269f, 0.00269151f, -0.136611f, 0.0220631f, -0.00476321f, 0.0281217f, 0.0243983f, -0.00436437f, 0.00491977f, 0.0540143f, 0.0410553f, -0.00945594f, -0.0711867f, -0.011407f, -0.0290617f, 0.0077444f, -0.0194761f, -0.0353022f, 0.0242323f, 0.121606f, 0.136937f, 0.117977f, 0.0648052f, 0.000369128f, -0.0286182f, -0.000851573f, -0.0675435f, 0.0374786f, 0.0108061f, -0.00134871f, -0.0419874f, 0.0271549f, -0.21822f, 0.268321f, -0.00535237f, 0.011111f, -0.0614932f, 0.0500974f, 0.0900748f, 0.0334851f, -0.101783f, -0.00498551f, -0.0075128f, 0.00031712f, 0.0485839f, 0.000919265f, 0.0326066f, -0.023036f, 0.0096988f, 0.0178391f, 0.0861196f, 0.0466213f, -0.0299909f, -0.0991148f, -0.0230341f, 0.334094f, -0.0382573f, 0.0395579f, -0.00590484f, 0.0206429f, 0.246985f, -0.0283786f, 0.0598143f, -0.0353774f, 0.091151f, 0.0944889f, 0.00249664f, 0.202462f, -0.00569812f, 0.00865333f, -0.00812537f, -0.188173f, -0.0627191f, -0.28001f, 0.00917071f, 0.0506412f, 0.0010405f, 0.0678395f, 0.16542f, -0.00219039f, 0.0110519f, -0.00379539f, 0.00535911f, -0.00791708f, -0.000717427f, -0.0325235f, 0.0842137f, -0.020968f, 0.192455f, 0.0856024f, 0.132173f, -0.00232728f, 0.0647325f, 0.104932f, -0.0235684f, 0.00335134f, 0.00515333f, 0.192284f, 0.0592319f, 0.143246f, -0.00214825f, -0.168829f, -0.0149753f, 0.00881463f, 0.00489184f, 0.0030815f, -0.0645487f, -0.236596f, 0.0211161f, 0.428909f, -0.0184283f, 0.150971f, -0.00403509f, 0.0892136f, 0.0527521f, -0.00892411f, 0.257531f, 0.0159127f, -0.0153799f, 0.0299046f, 0.00748111f, 0.02268f, -0.0283898f, -0.0224564f, -0.00329609f, -0.0642335f, 0.0385503f, 0.00387719f, -0.0795388f, 0.0385978f, 0.0338672f, -0.00181007f, 0.500546f, 0.0174027f, -0.00941603f, 0.00119533f, 0.161396f, 0.0277067f, -0.0113644f, 0.00243689f, 0.0240222f, 0.00074696f, -0.00329644f, 0.00571551f, 0.353842f, -0.0345694f, 0.0954816f, 0.022245f, 0.0639779f, -0.0209006f, -0.0100804f, -0.0223871f, 0.00248849f, -0.0231191f, -0.105286f, -0.0150994f, 0.00230265f, -0.0295301f, 0.0119341f, 0.00911531f, 0.0540066f, 0.0076047f, -0.0945892f, 0.0196067f, -0.0357786f, 0.0719775f, -0.0972845f, 0.142406f, -0.18177f, 0.00491428f, 0.000342362f, -0.0186926f, 0.0489506f, -0.0333847f, -0.017827f, -0.00585373f, 0.0250148f, -0.0496847f, 0.00595432f, 0.180951f, -0.0459607f, -0.0360709f, -0.168328f, -0.0724864f, -0.161582f, 0.0156965f, -0.0463856f, 0.00603378f, -0.0396591f, 0.100121f, 0.00849666f, 0.0438226f, 0.0247446f, 0.0309354f, -0.0876779f, -0.0223912f, 0.0149475f, -0.0619022f, -0.0198987f, 0.0258675f, 0.0760512f,
        0.0237833f, 0.00298876f, 0.0487694f, 0.00950606f, -0.074622f, 0.0192038f, -0.0202395f, 0.105125f, -0.0154085f, 0.0355691f, 0.00281225f, 0.00531638f, 0.0101454f, 0.0510713f, 0.0313131f, -3.24692e-05f, 0.0563302f, -0.00384794f, -0.0967057f, -0.00911184f, -0.034748f, -0.00885298f, -0.00145702f, 0.00841001f, -0.00386897f, 0.00954715f, 0.0060942f, -0.00779779f, 0.0341911f, 0.0373562f, 0.000677265f, -0.0620633f, 0.00208294f, -0.0215586f, -0.085074f, 0.0143441f, -0.0186877f, 0.00127867f, -0.01249f, -0.00504883f, -0.00104019f, 0.0121985f, 0.000512828f, -0.00772995f, 0.00468516f, -0.0139477f, -0.0211804f, 0.210879f, 0.00785329f, -0.000516933f, -0.00212956f, -0.0162727f, 0.00414868f, 0.0109553f, 0.000250999f, -0.00637749f, -0.00108913f, -0.00648906f, -0.0123977f, 0.0104616f, 0.0241319f, 0.0770632f, 0.00195405f, -0.00752428f, -0.0405081f, -0.0883033f, 0.0394711f, 0.0062544f, 0.0315002f, -0.0138193f, -0.0353362f, 0.00803457f, 0.0055575f, -0.00122304f, -0.00591179f, -0.000313378f, -0.00928775f, 0.00167335f, 0.00110711f, 0.0102733f, -0.0102128f, -0.0332447f, -0.0050578f, -0.0365285f, 0.00129188f, -0.00545454f, -0.0488076f, -0.0522689f, -0.0028496f, 0.0269232f, -0.00264586f, 0.00549725f, 0.0937312f, -0.0097157f, 0.000703438f, -0.0316939f, 0.00265145f, 0.00747435f, 0.00703635f, -0.0498706f, 0.0260258f, 0.00486406f, 0.00831138f, 0.00331964f, -0.0116462f, -0.000328743f, -0.0193854f, 0.012874f, -0.0140591f, 0.00294906f, 0.167637f, -0.00563081f, 0.00047881f, -0.0132155f, -0.088562f, -0.00763682f, 0.00861545f, 0.0484862f, 0.118604f, 0.00888342f, -0.0480975f, -0.0108402f, -0.00768345f, -0.214419f, -0.045855f, 0.000607434f, 0.00143275f, 0.000233664f, 0.00111974f, 0.0283561f, -0.0137152f, 0.035663f, -0.0231469f, 0.0205628f, 0.0685008f, 0.0106492f, 0.00590557f, -0.00685771f, 0.00424108f, 0.000113577f, 0.00595773f, 0.00665598f, 0.000441705f, -0.00402036f, -0.0262544f, 0.00611645f, 0.0116063f, -0.00424871f, 0.0342696f, 0.0381022f, -0.0588067f, -9.04306e-05f, 0.013434f, 0.0049054f, 0.0123942f, -0.000403249f, 0.0504587f, -0.00181204f, 0.00841684f, 0.0187689f, 0.0174106f, 0.00611652f, 0.00976013f, 0.000955711f, 0.00209072f, -0.0257193f, -0.0127599f, 0.00699173f, -0.0153516f, -0.00193625f, 0.0528177f, 0.0170662f, 0.0746572f, 0.00809554f, -0.027025f, -0.0257472f, -0.00256271f, -0.0890082f, -0.00221022f, -0.00891542f, -0.00903598f, -0.0144857f, 0.0554675f, -0.00986486f, 0.00189685f, 5.93501e-05f, 0.00462237f, 0.00532594f, 0.00433364f, -0.003124f, 0.04f, -0.000328486f, -0.0648411f, -0.00377033f, 0.139774f, 0.00230164f, 0.0115385f, 0.0125043f, 0.148022f, -0.0284796f, -0.00155402f, -0.00387695f, 0.00829478f, -0.0471497f, -0.0015643f, -0.00582674f, -0.00431319f, 0.000878919f, 0.00687072f, -0.00301133f, 0.00398096f, -0.00563914f, -0.0026393f, -0.00377055f, -0.0609272f, -0.118688f, 0.00517703f, 0.0836725f, -0.012182f, -0.0512972f, 0.0119928f, 0.0247734f, -0.0427426f, 0.0341825f, 0.0698612f, 0.00279914f, -0.00847926f, -0.0226391f, 0.020679f, -0.00144619f, -0.0104832f, 0.0195441f, 0.000150691f, 0.0815801f, -0.00616593f, 0.00379428f, -0.00447982f, 0.00261409f, 0.0600844f, -0.0213836f, -0.00804557f, 0.00325642f, 0.00854879f, -0.0814344f, -0.027769f, -0.00191851f, 0.00536533f, -0.0164033f, -0.00257131f, -0.00205376f, -0.0200541f, -0.0128954f, -0.00532982f, 0.0022407f, -0.00130887f, 0.00425618f, -0.00845818f, -0.00126148f, -0.0107566f, 0.00104842f, -0.00435674f, 0.00433842f, -0.0109865f, 0.000301519f, 0.00589863f, -0.00851759f, -0.00137109f, -0.0256632f, 0.0120122f, -0.00451766f, -0.0132172f, 0.0204377f, 0.00862719f, -0.00529603f, 0.0007616f, -0.00779072f, 0.000307369f, 0.0161384f, 0.0140168f, -0.00223271f, -0.0234216f, 0.00152691f, 0.00407567f, -0.00575267f, -0.0169706f, 0.00373715f, -0.0130443f, 0.0149063f, -0.00592504f, -0.00101738f, -0.00432452f, 0.00608682f, -0.00623923f, -0.0048846f, 0.00141049f, -0.00787022f, -0.00325903f, -0.00925192f, 4.10188e-05f, -0.00650579f, -0.00344007f, -0.00507379f, -0.010943f, 0.0033921f, 0.0262149f, -0.0109309f, -0.00218072f, 0.00487267f, -0.00424018f, 0.0190863f, -0.0205672f, -0.00521787f, -0.749656f, 0.0045255f, -0.0111087f, -0.00594957f, -0.00784532f, -0.00218566f, -0.00261733f, 0.00115839f, 0.00810127f, -0.00685174f, -0.000515265f, 0.00996413f, 0.00908507f, -0.010911f, 0.0199673f, 0.00424915f, -0.0168506f, -0.0127626f, -0.0068238f, 0.0141051f, -0.0106615f, 0.00332799f, 0.00636155f, -0.0260333f, 0.00595097f, 0.0191085f, -0.0049198f, 0.00793315f, -0.00309666f, 0.0137166f, -0.00473366f, 0.0127659f, 0.000838826f, 0.0352708f, -0.00566433f, 0.00439918f, 0.00403144f, -0.0103773f, 0.000578005f, -0.00181792f, -0.0300049f, -0.00661571f, 0.0085107f, 0.00894339f, 0.00861617f, 0.00351911f, 0.016009f, -0.00165849f, 0.00140448f, 0.00854556f, -0.000467159f, 0.00526625f, 0.0113457f, -0.000892589f, -0.00943319f, 0.016298f, 0.0129145f, 0.00977724f, -0.00864554f, -0.0149309f, 0.0109739f, 0.00925517f, 0.00301191f, -0.00253138f, -0.0198261f, 0.00383641f, 0.00511284f, -0.0561408f, -0.0281949f, -0.00444545f, -0.00338158f, -0.00161292f, -0.00978353f, 0.00446439f, 0.000485823f, 0.000591379f, 0.00729576f, -0.024535f, 0.00937071f, 0.00193014f, 0.00812366f, -0.015649f, -0.00101637f, 0.0112705f, 0.00182169f, -0.00906464f, 0.0080621f, -0.0130414f, -0.000293886f, -0.00548405f, -0.00557287f, -0.00444211f, 0.000131822f, -0.0116247f, 0.00918694f, 0.00706824f, -0.00459982f, -0.00134241f, 0.00769962f, -0.000905408f, -0.00643464f, 0.00195699f, 0.0103661f, 0.0117231f, 0.00141366f, 0.013737f, -0.00475491f, -0.00389627f, -0.008428f, -0.00336822f, -0.0123985f, -0.00384732f, -0.00772105f, -0.00399041f, 0.00441658f, -0.0179348f, 0.00088589f, 0.00130237f, -0.00910743f, -0.000932973f, -0.000705488f, -0.00845157f, -0.00409019f, -0.00198943f, -0.00037801f, -0.0110968f, -0.00639611f, 0.00967489f, -0.00286205f, -0.00142743f, 0.00952024f, 0.0067011f, -0.00771389f, 0.000101275f, 0.00173372f, 0.000959312f, 0.00841471f, 0.00336334f, 0.00371336f, 0.00482025f, -0.00711383f, 0.00583148f, 0.0108545f, -0.000470039f, -0.0110626f, 0.00324574f, 0.025979f, 0.0153801f, -0.00239289f, -0.0364105f, -0.0252222f, 0.00766028f, -0.000371992f, -0.00263989f, 0.0215774f, 0.0230998f, -0.00223724f, -0.000281751f, -0.00482297f, -0.0175295f, -0.00712851f, 0.0106509f, 0.00430235f, 0.00410187f, 0.00823292f, 0.00280169f, 8.28998e-05f, -0.00169138f, -0.00976853f, -0.00530213f, -0.00814388f, 0.0013187f, 0.00816157f, 0.00138731f, -2.68979e-05f, -0.0103893f, -0.0500543f, 0.000847671f, 0.00327953f, 0.00418289f, 0.0180997f, -0.00027566f, -0.00544788f, -0.0076323f, -0.00551657f, -0.00599236f, -0.0127374f, -0.0174632f,
        -0.000449777f, -0.000137405f, -0.0762075f, 0.000949166f, 0.0346124f, -0.0111424f, 0.0108357f, 0.0121679f, 0.0242749f, 0.052692f, -0.0017713f, 0.0053728f, 0.0128862f, -0.0162366f, 0.0125041f, -0.00602398f, 0.0107778f, -0.00323086f, -0.00914208f, -0.013884f, 0.00755173f, -0.0175622f, 0.00473339f, -0.015003f, -0.0238219f, 0.004502f, 0.00187154f, 0.0041163f, -9.36184e-05f, 0.00873372f, 0.0121869f, -0.020973f, -0.006006f, -0.0038208f, 0.00210471f, 0.00255549f, -0.0251856f, -0.0626372f, -0.0059258f, -0.0058662f, -0.0946306f, 0.00197436f, 0.00105865f, -0.0033595f, 0.0158977f, -0.0036025f, -0.00568902f, -0.0202577f, -0.000251319f, -0.0117895f, -0.0144239f, -0.0144024f, -0.0150431f, -0.0354826f, -0.0135123f, -0.000422157f, 0.0286438f, -0.000884989f, -0.00675718f, 0.013241f, -0.0118388f, 0.0321394f, -0.000803071f, 0.11408f, -0.00806301f, -0.00831608f, 0.0165189f, 0.016094f, -0.000449332f, -0.00695901f, 0.0437514f, -0.00172117f, 0.00180391f, -0.000859933f, -0.0144826f, 0.0262613f, -0.00194352f, -1.98829e-05f, -0.00902827f, -0.00400867f, -0.00600827f, 0.0120846f, -0.0162493f, 0.0418596f, 0.00131911f, -0.00631566f, 0.00270484f, -0.0950513f, 0.00726431f, -0.0169798f, -0.000554365f, -0.00256903f, -0.00885843f, 0.0104025f, 0.00590779f, -0.00175832f, 0.0168603f, 0.00964353f, -0.0180614f, 0.0213157f, 0.0209548f, -0.0231143f, -0.00121617f, -0.0129815f, -0.0199287f, 0.00863336f, -0.00464991f, 0.0162288f, -0.340115f, -0.011018f, -0.0593997f, 0.00644821f, 0.0416332f, 0.0394596f, 0.0172296f, 0.00494231f, 0.0143805f, -0.00819845f, 0.00196982f, 0.00393258f, 0.0246168f, -0.0235927f, 0.0131416f, -0.0190432f, -0.0237865f, -0.0155627f, 0.0265165f, 0.0162884f, 0.00321098f, 0.0136674f, -0.000966112f, -0.0100813f, -0.00604589f, 0.00889466f, 0.0113945f, 0.0264707f, 0.00371883f, -0.00843358f, 0.0145675f, 0.0048638f, 0.00110399f, -0.00130233f, 0.00740726f, -0.00393368f, -0.0242178f, 0.00341681f, 0.00115369f, -0.00297881f, -0.0844071f, 0.0537151f, -0.00209399f, 0.0310295f, 0.0383914f, 0.00456459f, 0.0188114f, -0.0177144f, 0.0133258f, 0.0584683f, -0.00640495f, 0.0175946f, 0.0186782f, 0.00213311f, 0.00393403f, 0.00382759f, 0.00267507f, 0.00493673f, -0.00856695f, -0.00627955f, -0.0103436f, -0.000671664f, -0.110419f, 0.0307264f, 0.0042176f, 0.0031638f, 0.0154172f, 0.00265482f, 0.0410853f, 0.00833895f, -0.0183989f, -0.000717906f, -0.0090387f, -0.00404523f, -0.00976238f, -0.0137555f, 0.000157289f, -0.00341186f, -0.0214878f, 0.0142639f, 0.00624623f, 0.000537292f, -0.0520912f, -0.0432221f, -0.00330415f, 0.0263942f, -0.00150974f, 0.00172088f, -0.0815726f, -0.0201155f, -0.00986346f, 0.0121252f, 0.00198959f, -0.0349936f, -0.00608366f, -0.00399543f, 0.0192487f, -0.0123156f, 0.0072797f, 0.000507143f, 0.0334805f, 0.000609379f, 0.00961966f, -0.00697663f, 0.00201967f, -0.0207349f, -0.0103385f, -0.00343849f, -0.00330492f, 0.035106f, -0.00456996f, 0.00197528f, 0.016148f, 0.0142903f, 0.0616483f, 0.0093118f, -0.0596028f, 0.00945764f, -0.00659242f, 0.118389f, -0.00259384f, -0.00285344f, 0.00567036f, 0.0195813f, -0.00461807f, -0.0608699f, 0.00380259f, 0.00143385f, -0.00466997f, 0.0194046f, -0.0198423f, -0.00334569f, -0.014399f, 0.0130021f, -0.0141619f, -0.00859914f, 0.00997122f, -0.0198446f, -0.0094162f, -0.0116609f, -0.0111888f, -0.00903524f, 0.00937981f, 0.01772f, -0.00236374f, -0.00870162f, 0.000141193f, -0.0343695f, -0.00997931f, 0.0073531f, -0.100394f, -0.00367661f, -0.00124499f, 0.00318026f, 0.0554203f, -0.00342582f, -0.0104147f, -0.0577869f, -0.0126485f, -0.0332496f, 0.0346141f, 0.0307962f, -0.0174745f, -0.0387339f, 0.0167707f, -0.0363424f, 0.0154902f, -0.0118644f, -4.63543e-06f, -0.0683506f, -0.0344076f, -0.00104884f, -0.00883997f, -0.00305185f, -0.0150299f, -0.0186403f, 0.0110238f, 0.00779224f, -0.0102231f, 0.0087488f, -0.0138988f, -0.0229105f, -0.0244903f, -0.0202919f, 0.00135903f, -0.00574432f, 0.00254918f, 0.0340209f, -0.046428f, -0.00670622f, 0.000925543f, -0.0249251f, -0.00275456f, 0.0199177f, 0.000210993f, 0.027762f, -0.0228046f, 0.0484813f, 0.00538959f, 0.0136714f, -0.00690097f, -0.0448533f, -0.00815204f, 0.00734891f, 0.0173959f, -0.0379109f, 0.0594617f, -0.00722084f, 0.0415935f, 0.014792f, -0.0170252f, -0.0139396f, 0.00146415f, 0.00117702f, 0.0685559f, 0.00727832f, -0.107566f, -0.0112505f, 0.033853f, 0.0046957f, -0.0242369f, 0.0148181f, -0.0723487f, -0.00961667f, 0.0304085f, -0.00520772f, -0.0316467f, 0.0327801f, -0.00755137f, 0.0166041f, -0.0557288f, -0.0227759f, -0.00314548f, 0.0152585f, 0.020071f, -0.0377076f, 0.00687613f, -0.0273935f, -0.00647955f, 0.0105047f, -0.0137238f, 0.023264f, -0.0455722f, -0.00221414f, -0.0258535f, -0.0236395f, 0.0593407f, 0.00448763f, 0.0150777f, 0.00437925f, 0.0295782f, -0.0344752f, 0.00365267f, 0.140464f, -0.0479012f, 0.025726f, 0.119063f, 0.000301925f, -0.00810565f, -0.354073f, -0.0723185f, -0.0046123f, 0.033882f, -0.044552f, -0.0138361f, 0.00384129f, 0.0139111f, -0.01667f, -0.0821503f, 0.0029974f, -0.0306725f, 0.0160366f, 0.0334754f, 0.0192693f, -0.00616713f, -0.00232275f, 0.0107987f, 0.00437057f, 0.0017298f, 0.0196916f, -0.0417255f, -0.00911193f, 0.00876709f, -0.00172422f, -0.00105248f, -0.0191631f, -0.00387423f, -0.0102766f, -0.025317f, -0.0416204f, -0.0319611f, -0.00359193f, 0.00424064f, -0.00575092f, -0.0282402f, 0.0745899f, -0.0126492f, -0.0162564f, -0.261967f, -0.705265f, -0.0403731f, -0.00209634f, -0.694297f, 0.00956909f, 0.0158826f, 0.0130207f, 0.003825f, -0.000300812f, -0.0121346f, 0.00642053f, -0.012902f, 0.0309272f, 0.0609192f, -0.00654145f, -0.0937578f, -0.00432024f, -0.00767539f, 0.0461248f, 0.00701077f, -0.0174477f, 0.00563833f, -0.0107107f, -0.0255275f, 0.00892488f, -0.00166062f, 0.039829f, -0.00150394f, 0.00742194f, -0.00885529f, -0.0103532f, 0.0777858f, 0.0885367f, -0.00425715f, 0.0423651f, -0.0446651f, -0.635069f, -0.00919329f, -0.00356176f, 0.00988705f, 0.0116529f, -0.0401253f, 0.00260105f, 0.00573955f, -0.0667439f, 0.101175f, 0.0765288f, -0.0120077f, 0.00322599f, -0.0192768f, 0.0382749f, -0.222119f, -0.0452036f, 0.0424303f, 0.0890699f, 0.0117557f, 0.0315167f, 0.0284256f, 0.00541845f, -0.250147f, 0.00420668f, -0.0189724f, -0.00416381f, -0.00162803f, -0.0108763f, -0.00970892f, 0.0134476f, -0.0254931f, 0.0307225f, 0.00128596f, 0.0171106f, 0.00467854f, -0.0124376f, 0.0183396f, 0.0021754f, 0.00170886f, -0.0226898f, 0.0250111f, -0.0533301f, -0.0163268f, 0.00618995f, 0.0416378f, 0.0475397f, 0.0105684f, -0.00440933f, 0.0496722f, -0.0215733f, -0.0256361f, -0.0285091f, -0.0276881f, -0.00102202f, -0.0720219f, -0.0296656f,
        0.00465617f, 0.00138814f, -0.0913312f, -0.0161213f, 0.0160887f, 0.0204469f, -0.0223319f, 0.015304f, 0.000397867f, 0.00824013f, 0.0114613f, 0.00408309f, 0.0384456f, -0.00453968f, 0.0176576f, 0.100434f, -0.0393971f, 0.0160015f, -0.00313166f, -0.0058054f, 0.0342083f, 0.0333727f, 0.00275399f, -0.0111393f, -0.0656798f, 0.0117794f, 0.00399766f, 0.00310487f, 0.00290905f, 0.00311256f, 0.0103328f, 0.00221549f, -0.00340486f, -0.00955604f, -0.010614f, 0.0144013f, -0.0244803f, 0.246714f, 0.00585756f, -0.0183366f, 0.0131221f, -0.015529f, 0.0634503f, -0.00107566f, 0.0230663f, -0.00523926f, -0.0100486f, -0.0270644f, 0.0938544f, -0.0136558f, 0.0164469f, -0.349288f, 0.0108305f, 0.0621752f, -0.00813808f, -0.0218271f, 0.0168811f, -0.00509217f, -0.0249135f, 0.0268669f, -0.0294336f, 0.0396944f, -0.00419361f, 0.00843219f, -0.000475472f, -0.0122415f, 0.0142385f, 0.0240099f, -0.0041296f, 0.0167314f, -0.0210217f, -0.00275032f, 0.0121842f, -0.00556776f, -0.0215306f, 0.0411878f, -0.00102203f, 0.00011487f, -0.0142263f, -0.00257424f, -0.0044306f, 0.0115836f, -0.0331884f, 0.0153153f, 0.0023461f, -0.0229996f, -0.00982945f, 0.0207273f, 0.0039542f, -0.0275622f, -0.00118208f, -0.00703868f, -0.0111554f, 0.0155981f, -0.0197133f, -0.00157645f, 0.0790344f, 0.0277319f, -0.0239723f, 0.0133704f, 0.0153687f, -0.0220235f, -0.0652554f, 0.0340702f, -0.0256995f, 0.00463251f, -0.134567f, 0.0048301f, -0.0935251f, -0.0125128f, -0.0560035f, -0.000903825f, 0.0231884f, 0.0678238f, 0.0172834f, 0.0226948f, -0.00784814f, -0.000168366f, 0.0165854f, 0.00979108f, -0.010978f, -0.147669f, 0.020833f, -0.0320907f, -0.339001f, -0.0307849f, -0.00796792f, 0.00704321f, -0.0258511f, 0.0302859f, -0.0174755f, -0.0208662f, -0.00800382f, -0.00772683f, 0.00787931f, 0.0244046f, 0.0635711f, -0.0490687f, 0.00843431f, -0.00969577f, -0.00403176f, -0.00225678f, -0.00425568f, 0.00423476f, -0.0522863f, 0.00901175f, 0.00701737f, 0.0203201f, 0.00764967f, -0.0128627f, -0.0154611f, -0.00973917f, 0.0172989f, 0.00679487f, -0.00897315f, -0.00337138f, -0.0103584f, -0.00507785f, -0.00390477f, 0.0472275f, 0.0060846f, 0.0151745f, 0.0472687f, 0.000490868f, 0.0196255f, 0.00541134f, -0.0206129f, -0.00112977f, -0.0197924f, -0.0553976f, -0.098063f, 0.0664134f, 0.00349375f, 0.00311233f, 0.0401445f, 0.0128354f, -0.0250036f, 0.0436594f, -0.0462325f, -0.00102946f, -0.013474f, -0.0172785f, 0.0394013f, -0.00569089f, 0.000160535f, 0.000504291f, 0.0504433f, -0.0205918f, 0.0101148f, -0.00946464f, -0.0885629f, -0.04032f, -0.012075f, 0.492342f, -0.000999111f, 0.00407901f, 0.0888248f, 0.0100317f, -0.024372f, -0.0211601f, 0.000658811f, -0.0209988f, -0.0190039f, -0.0219266f, -0.0516314f, -0.00642571f, 0.00488745f, 0.00512097f, 0.0145898f, -0.00157307f, 0.0026168f, 0.0156606f, -0.00531944f, -0.017507f, -0.0180003f, 0.00282254f, 0.0143295f, 0.0777137f, -0.00385748f, -0.00549398f, -0.0172826f, 0.0323722f, 0.185825f, 0.0121615f, 0.00399867f, -0.0541097f, 0.0386216f, 0.0595922f, 0.594257f, -0.00955271f, 0.00343269f, 0.0139925f, 0.00328999f, -0.0792421f, -0.045498f, 0.0113837f, -0.00976291f, 0.00624078f, -0.0254107f, -0.0216194f, -0.028773f, 0.0236943f, 0.0197444f, -0.00939094f, 0.0135671f, -0.0407697f, 0.00794318f, -0.0184558f, -0.0282076f, -0.0112124f, 0.00710705f, 0.0203747f, -0.00201855f, -0.0137849f, -0.00224183f, -0.00758043f, 0.0109492f, 0.0111736f, -0.0524165f, -0.00359813f, -0.0105491f, 0.00795013f, 0.0490089f, -0.0172285f, -0.131601f, -0.640844f, -0.00210558f, -0.0191391f, 0.144537f, -0.0187546f, -0.0117677f, -0.0243942f, -0.0673674f, 0.0116665f, -0.00634048f, -0.0171121f, -0.018849f, -0.0452217f, -0.0314511f, 0.01823f, -0.0338747f, -0.00232084f, -0.0184449f, -0.0628265f, -0.00846206f, 0.00285066f, 0.281056f, -0.0109403f, -0.036282f, 0.00725135f, -0.027479f, -0.0120889f, 0.0185699f, -0.00228023f, 0.000971992f, 0.020036f, -0.0437852f, -0.013831f, 0.0284799f, -0.0116033f, -0.0213317f, -0.0391473f, -0.0180216f, 0.0224665f, 0.00661723f, 0.0188164f, -0.00856477f, -0.0188785f, -0.0419517f, -0.0383142f, 0.00822795f, -0.0210551f, 0.0376673f, -0.0158509f, 0.0531296f, -0.0222652f, 0.0202294f, 0.0377989f, -0.0486931f, -0.0236611f, -0.0364076f, -0.0364403f, 0.105507f, -0.0520728f, -0.085646f, -0.0517868f, 0.00898522f, 0.0145328f, -0.0152412f, 0.00230019f, -0.0490983f, 0.0199105f, 0.193699f, -0.00652485f, -0.0293521f, -0.101157f, 0.00759732f, 0.0611226f, 0.00668415f, -0.0644944f, -0.00138395f, -0.0872389f, -0.0289147f, -0.0104552f, 0.0102965f, -0.00918203f, -0.0163947f, 0.00688836f, -0.0460991f, 0.0010579f, -0.0220147f, 0.00389295f, -0.0450669f, -0.0338309f, -0.00643917f, -0.164896f, 0.00520622f, -0.00943891f, 0.015696f, -0.0488516f, 0.00357405f, 0.395393f, 0.0142406f, 0.0375136f, 0.0266987f, 0.00442581f, -0.0355697f, 0.0566785f, -0.0609618f, 0.0953531f, 0.0234361f, -0.0235014f, -0.0201052f, 0.0185904f, 0.0944014f, -0.00254259f, 0.0149094f, -0.00267577f, -0.0236442f, 0.0304207f, 0.0195184f, 0.00453831f, -0.010829f, -0.00384567f, -0.00720987f, 0.00142745f, 0.00339592f, 0.0255406f, -0.0328377f, -0.0418446f, 0.00524565f, -0.019943f, -0.00744414f, -0.0262656f, -0.00295384f, -0.012041f, 0.00168772f, -0.0393009f, -0.0333347f, -0.0127033f, -0.0399219f, -0.12722f, -0.223577f, 0.0811929f, -0.130626f, -0.0705225f, 0.174048f, 0.0435034f, -0.136602f, 0.00640297f, -0.166342f, 0.0597288f, 0.0182928f, 0.00638083f, 0.00566142f, 0.0143743f, -0.0117229f, -0.00092003f, -0.00302193f, 0.0193828f, 0.0549159f, -0.01403f, -0.0686686f, -0.00131562f, -0.0395576f, 0.0140634f, 0.00728921f, -0.0222314f, 0.0847774f, 0.00397858f, -0.037106f, 0.00703206f, 0.0217107f, 0.026982f, -0.0970178f, 0.00170535f, 0.00461989f, -0.0484043f, 0.0549405f, -0.00663961f, -0.0301618f, 0.0402775f, -0.126174f, 0.042974f, 0.00767555f, -0.0323881f, -0.0021808f, 0.00152122f, -0.0794255f, 0.00950137f, 0.00617034f, -0.186531f, 0.0667047f, 0.158624f, -0.0498641f, 0.000181888f, -0.00194408f, 0.0130678f, -0.0624929f, 0.099144f, 0.00810417f, 0.174436f, 0.0147924f, 0.00815054f, 0.0152255f, -0.0833151f, -0.072767f, -0.201512f, -0.0109339f, -0.003133f, -0.00430304f, -0.0208616f, -0.0187232f, 0.0277294f, -0.451013f, 0.0336152f, -0.00462652f, 0.00806012f, -0.000483294f, 0.0313363f, 0.0948398f, -0.0302999f, -0.00779582f, -0.0975373f, 0.0429978f, -0.0117262f, -0.00451523f, -0.0175741f, 0.0914118f, 0.0390275f, 0.00306197f, 0.0172763f, 0.0486995f, -0.0628708f, -0.00845093f, 0.00565009f, -0.0126375f, 0.0362389f, -0.0893211f, -0.0264466f,
        0.0309426f, -0.0247239f, -0.0618656f, -0.16444f, 0.0416493f, -0.0039234f, -0.0446445f, -0.0806408f, 0.0315374f, -0.0123988f, 0.0385759f, 0.0315165f, 0.00742563f, -0.0276244f, 0.013597f, -0.000546713f, -0.126003f, -0.0403999f, -0.0199147f, 0.090123f, 0.0122743f, 0.0904552f, 0.0480448f, -0.0274991f, -0.0463688f, 0.132874f, -0.0163207f, 0.00931698f, 0.00050237f, -0.034227f, 0.0273549f, 0.0257694f, 0.0545361f, -0.0196519f, -0.00616926f, 0.0252382f, 0.00394299f, 0.00503618f, -0.000107687f, -0.00739968f, 0.0155088f, -0.0271828f, 0.0136159f, -0.0184294f, 0.00419291f, -0.0705982f, 0.00832841f, -0.0455188f, 0.0203078f, -0.0104058f, -0.00448528f, 0.0346675f, 0.00227903f, 0.0283768f, 0.0146701f, 0.0238016f, -0.0041065f, -0.00951874f, -0.0656203f, 0.00289312f, -0.0280637f, 0.064775f, -0.0145084f, -0.0166982f, 0.112919f, -0.030709f, -0.08767f, 0.0231176f, -0.00683745f, 0.145201f, -0.0588483f, -0.00211676f, 0.0707442f, -0.0175353f, 0.0425204f, 0.047214f, -0.00454212f, 0.108341f, -0.0655429f, -0.0661698f, -0.00742549f, 0.0525604f, -0.00200138f, 0.0760939f, 0.0208251f, -0.0183413f, -0.019956f, 0.0497461f, -0.00312012f, -0.026077f, -0.00492334f, -0.0389153f, -0.0240003f, -0.0236527f, -0.00949685f, 0.00834218f, 0.196113f, -0.0203076f, -0.0373067f, 0.0511745f, -0.000502779f, -0.0506356f, 0.0270005f, 0.0560514f, -0.0566957f, 0.00592365f, -0.0950855f, 0.0330845f, 0.0126008f, -0.0178738f, 0.00655207f, -0.00560155f, 0.0226922f, 0.122885f, -0.0227311f, -0.0185407f, -0.024025f, 0.000734875f, -0.0501656f, 0.00259467f, -0.0401208f, -0.00270448f, 0.0298842f, -0.0449168f, -0.083653f, -0.0667249f, -0.012424f, 0.0228182f, -0.0256871f, 0.0103425f, 0.00584589f, -0.0313978f, -0.00512387f, -0.0389378f, 0.00783504f, 0.0246462f, 0.0204282f, -0.0313174f, 0.0293227f, -0.0135298f, 0.0250816f, 0.00154453f, 0.00455047f, 0.0664336f, -0.0924272f, 0.0141598f, 0.0249505f, 0.0114919f, 0.127537f, -0.0302333f, -0.0464173f, 0.0312457f, 0.0119746f, 0.00862732f, -0.0221585f, -0.00284848f, 0.014157f, 0.0253277f, 0.00495452f, 0.00886403f, 0.00389645f, -0.0347684f, -0.0039163f, 0.0218669f, -0.0417104f, 0.00547612f, -0.013528f, -0.00265715f, 0.180858f, -0.000752272f, -0.18944f, 0.0260848f, -0.000632882f, 0.0126054f, 0.0359676f, 0.0302849f, -0.0371376f, 0.0941217f, -0.0281283f, -0.0280773f, -0.011986f, -0.0406752f, 0.239648f, -0.00517518f, -0.00410975f, 0.00103368f, 0.0209206f, -0.0476301f, 0.00454544f, -0.0149667f, -0.0314583f, -0.00242636f, -0.0512553f, 0.0608112f, 0.0428258f, 0.0173526f, 0.0602241f, -0.0548611f, -0.131965f, -0.0495486f, 0.00765915f, -0.062264f, -0.000979455f, -0.0652348f, -0.147691f, -0.0231597f, 0.0251012f, -0.0946399f, 0.0277068f, -0.00621829f, 0.0313192f, 0.0259072f, 0.00394534f, -0.0118648f, 0.004981f, 0.0594206f, -0.0358001f, -0.0710233f, -0.00969833f, 0.023656f, -0.0388052f, -0.00855584f, 0.259141f, 0.0142973f, -0.00158563f, 0.0164536f, 0.0212657f, 0.00174633f, 0.0514006f, -0.00881672f, 0.0221807f, 0.0413859f, 0.0143335f, -0.163744f, 0.236609f, 0.0189168f, -0.0167902f, 0.0688642f, -0.0370002f, -0.0330411f, -0.0653769f, 0.00270779f, -0.00759605f, -0.0221796f, 0.0385442f, -0.0446415f, 0.06948f, -0.033133f, -0.0352207f, -0.0310347f, 0.00721417f, 0.0857527f, 0.00283876f, -0.115239f, 0.0347347f, -0.0365242f, 0.0587821f, 0.00664576f, -0.0273541f, -0.016766f, -0.0138301f, 0.00564337f, 0.0364023f, -0.0560315f, -0.0449002f, -0.0932135f, -0.0177926f, -0.0494535f, 0.0610707f, -0.00528969f, 0.114377f, -0.0275389f, 0.0177389f, -0.0280061f, -0.00589614f, -0.00858413f, 0.0105453f, -0.0247948f, -0.0472122f, -0.000931705f, -0.0574841f, -0.0412944f, 0.00216405f, -0.0681429f, -0.00229429f, 0.00222781f, -0.0102497f, -0.0110639f, 0.0254925f, 0.0135797f, -0.0289002f, 0.00603638f, 0.0356664f, -0.0870163f, 0.552476f, 0.0106117f, -0.025193f, -0.0567232f, 0.00731144f, -0.00597211f, 0.00564131f, -0.037914f, 0.00553956f, 0.0244306f, 0.0163081f, 0.0614898f, -0.0103462f, -0.0125773f, -0.0129543f, -0.0425792f, -0.00984468f, 0.0241087f, 0.0391885f, 0.0113726f, 0.0740247f, -0.0314575f, 0.0847706f, 0.00766129f, -0.00782563f, -0.00219977f, 0.0364213f, 0.00561357f, -0.0207095f, -0.0389947f, -0.0574235f, -0.0215928f, 0.0242519f, 0.0150763f, 0.00640004f, 0.0049859f, -0.0883498f, 0.0259088f, -0.00976872f, -0.0257561f, -0.145433f, 0.0186583f, -0.0313577f, 0.0232484f, 0.135472f, -0.0611472f, -0.0134871f, -0.0152308f, 0.0481365f, -0.000509527f, 0.0241717f, -0.0205968f, -0.0464828f, 0.00742741f, -0.0585818f, 0.0174123f, -0.032865f, 0.0399474f, 0.0189778f, 0.0185407f, -0.0144228f, 0.0195944f, 0.0105867f, 0.0108527f, 0.0318328f, -0.07468f, 0.0640258f, -0.0166149f, -0.0161666f, 0.0270572f, -0.00831346f, 0.0213354f, -0.0331297f, 0.0314013f, -0.0295451f, -0.0309544f, 0.00883464f, -0.000784053f, 0.00228157f, 0.030596f, -0.0169894f, -0.0723077f, 0.0142356f, -0.042197f, -0.0273198f, 0.0607149f, 0.0824823f, 0.0722077f, -0.0207748f, -0.0090944f, 0.0268541f, 0.0273479f, 0.00481306f, -0.00487477f, -0.0183224f, -0.0126787f, 0.0311318f, -0.0985153f, -0.0152497f, 0.00489618f, -0.0141078f, -0.0060658f, -0.000568589f, -0.032613f, 0.00976906f, -0.0462634f, -0.0259696f, -0.0786609f, -0.0153404f, 0.0249492f, 0.00292531f, -0.0255124f, 0.0202219f, 0.0304817f, -0.0177191f, -0.0135411f, -0.0064023f, 0.048916f, 0.0348483f, -0.00747575f, 0.0256531f, -0.0264167f, -0.027836f, 0.026632f, -0.0408624f, 0.0405082f, 0.0435032f, -0.0481381f, 0.0232822f, 0.0406269f, -0.104934f, 0.032984f, 0.00642478f, -0.0123055f, 0.0323379f, 0.0262914f, -0.00313157f, -0.0307961f, -0.059502f, 0.043095f, -0.0842975f, -0.0634201f, -0.0069968f, -0.0269704f, 0.0525556f, -0.0145985f, -0.026517f, 0.0287775f, -0.00225143f, 0.00998218f, -0.0208695f, 0.00038333f, -0.0179813f, 0.0299511f, -0.0270286f, -0.0215702f, 0.00986492f, -0.121571f, 0.0374826f, 0.0280122f, -0.0349332f, 0.00798409f, 0.00126605f, 0.0544963f, -0.00189064f, -0.0770879f, -0.00792704f, 0.0613617f, 0.0133352f, 0.0303873f, -0.000380032f, 0.0189077f, -0.0194632f, -0.00659714f, -0.0571043f, 0.041608f, -0.0141942f, 0.012823f, 0.00537086f, 0.000970999f, 0.0332154f, 0.0570762f, -0.0137126f, 0.0101087f, -0.00108052f, -0.0265809f, -0.0247709f, -0.00362676f, -0.0148946f, 0.013131f, -0.00308769f, -0.158096f, 0.00257066f, -0.0143705f, 0.0888035f, 0.00916709f, 0.00514034f, -0.0227268f, 0.134988f, -0.0492885f, 0.0022784f, -0.0144922f, 0.0256463f, 0.0246127f, -0.0242015f, -0.0270194f,
        0.0236487f, -0.00133765f, -0.023996f, 0.0121123f, 0.0473768f, -0.0229827f, 0.0620781f, 0.0348273f, 0.0118778f, -0.0358558f, -0.00418959f, 0.026328f, 0.00159447f, -0.0285201f, 0.0242085f, 0.024281f, -0.120022f, 0.00322402f, -0.00124464f, -0.00395719f, 0.00586048f, 0.0264264f, 0.0202582f, -0.0172882f, 0.0167585f, 0.00926656f, 0.00103096f, 0.00249462f, 0.00288184f, -0.00771514f, 0.0255329f, 0.0516628f, -0.0170072f, -0.00388561f, -0.00997277f, 0.0355019f, 0.000978238f, -0.144348f, -0.00646585f, -0.013882f, 0.033804f, -0.0377087f, 0.00771159f, -0.0061665f, 0.0237085f, -0.0122598f, 0.0771705f, -0.0542605f, -0.0292168f, -0.0110855f, 0.00780249f, -0.0262439f, -0.0170252f, 0.0232333f, 0.0221474f, -0.000682905f, 0.0456239f, 0.00516233f, -0.0356498f, 0.0433573f, -0.0725911f, 0.122393f, -0.000836771f, 0.0154195f, -0.00217232f, -0.0458872f, 0.0576701f, 0.0347757f, 0.00437707f, 0.0167836f, -0.024089f, 0.00395376f, 0.0226754f, -0.000325613f, -0.0119747f, 0.0166885f, 0.0133881f, -0.00825686f, -0.0115485f, -0.0256805f, -0.013069f, 0.029991f, -0.0104672f, 0.0468771f, 0.018202f, -0.0499781f, -0.0150365f, 0.0351706f, 0.000881884f, 0.0257364f, -0.00567146f, -0.0125245f, -0.00638529f, 0.00949407f, -0.00206895f, -0.00294736f, -0.00599403f, 0.0100478f, -0.0708312f, 0.0164853f, -0.00509979f, -0.0820398f, 0.00301894f, -0.011352f, -0.103304f, 0.0361376f, -0.00276168f, 0.0140668f, 0.0182486f, -0.0224722f, 0.00670642f, -0.00173934f, -0.0763404f, 0.00545386f, -0.0451032f, 0.258199f, -0.000526159f, -0.00244376f, -0.0070213f, 0.0136966f, 0.00651444f, 0.00336226f, 0.0129456f, -0.00535145f, -0.0337439f, -0.0488545f, 0.0363396f, -0.000131419f, -0.0442874f, -0.00468587f, -0.00406768f, -0.0170205f, -0.0192772f, -0.00277597f, 0.0212662f, 0.0767458f, -0.0198272f, 0.00671115f, 0.00387314f, -0.00222632f, 0.017668f, -0.0152864f, -0.00217823f, -0.0302261f, 0.0201784f, 0.00912841f, 0.0418803f, 0.00397826f, -0.0171634f, 0.0562426f, -0.00595202f, 0.0317872f, 0.00277863f, -0.0198806f, -0.0105047f, -0.0078311f, -0.00416702f, 0.0284072f, 0.00135271f, 0.00845078f, 0.0125683f, -0.00724979f, 0.0567957f, 0.0255109f, 0.002417f, 0.0114722f, -0.0229208f, 0.00542141f, 0.000680912f, -0.0124263f, -0.0973681f, 0.0429572f, -0.00896565f, 0.00102447f, 0.0209145f, 0.0365617f, 0.00698999f, 0.0611891f, -0.0021814f, -0.00791606f, 0.0636013f, -0.0503155f, 0.041678f, -0.00722059f, -0.00547887f, 0.00243705f, -0.0177814f, -0.12321f, 0.0569086f, -0.00487058f, 0.0123446f, 0.0015868f, -0.0272469f, 0.0180903f, 0.0104843f, 0.0105209f, 0.00808024f, -0.0662313f, -0.0499085f, -0.0297908f, 0.00678693f, 0.0158422f, -0.0149847f, -0.212685f, -0.029142f, -0.0216139f, 0.0197027f, -0.00509483f, 0.0406666f, -0.00101148f, 0.0137954f, 0.0292058f, 0.0261623f, 0.0879647f, -0.0120199f, 0.0276628f, -0.00208332f, 0.00630364f, -0.00283301f, 0.0313885f, 0.00132789f, 0.00430711f, 0.131565f, 0.00856252f, -0.0451589f, 0.0151607f, -0.00609563f, 0.104563f, 0.0503204f, -0.00188153f, -0.00152094f, 0.0331939f, -0.0268272f, -0.0720271f, 0.0120254f, 0.00428272f, -0.010781f, -0.0235618f, -0.0599427f, -0.0128298f, -0.039684f, 0.0124311f, -0.00907946f, -0.0219339f, -0.00574204f, 0.00290369f, -0.0397143f, -0.0306637f, 0.0046412f, -0.102802f, 0.02052f, 0.0177221f, -0.000307451f, -0.663219f, -0.00099111f, -0.00863413f, -0.0648291f, 0.141571f, -0.0264896f, -0.00967159f, -0.0105556f, 0.00667919f, 0.019933f, -0.0081883f, -0.0256497f, -0.0425081f, -0.00260382f, -0.00437219f, 0.0181059f, 0.0588014f, -0.0156841f, -0.0992774f, 0.0577409f, -0.0112435f, 0.0118955f, -0.01259f, -1.68039e-05f, -0.0231843f, -0.0715207f, 0.00562568f, 0.00659099f, -0.00432696f, 0.0402245f, -0.0132643f, 8.8306e-05f, 0.00698941f, -0.0695019f, -0.0112349f, 0.0696259f, -0.142201f, -0.0227633f, -0.019462f, -0.0518398f, -0.0213576f, 0.0148991f, 0.0344155f, -0.0131575f, -0.012708f, -0.00177817f, -0.00639755f, -0.000887201f, -0.0257106f, -0.0247181f, 0.00548285f, 0.0290425f, 0.122557f, -0.00347772f, 0.0268244f, -0.00612725f, -0.0196236f, -0.0472946f, 0.00890478f, 0.000844572f, 0.0154442f, 0.024701f, -0.0306896f, 0.0231992f, 0.0425512f, -0.0302086f, 0.0319046f, 0.0310391f, -0.00796268f, -0.0411025f, 0.00749199f, -0.0374908f, -0.0108962f, 0.0293042f, 0.00369268f, -0.0138972f, -0.00285899f, -0.0473339f, 0.00105261f, 0.0269907f, -0.0314717f, -0.0538936f, 0.0837861f, -0.0145771f, 0.0345362f, 0.222726f, -0.034146f, -0.0154113f, 0.0519213f, 0.0351403f, -0.0609869f, 0.0181544f, -0.0165051f, 0.00702428f, -0.0109979f, -0.00444243f, -0.018915f, -0.027162f, 0.00253407f, 0.0133815f, -0.000469394f, 0.109107f, 0.0153356f, 0.00683112f, 0.0128685f, 0.0282692f, -0.0384653f, 0.000389417f, 0.106818f, 0.0799349f, 0.0567321f, 0.0479257f, 0.00394279f, -0.00575818f, -0.575371f, -0.0118667f, 0.00356253f, -0.0399865f, -0.0217626f, -0.019511f, 0.0108772f, 0.0134627f, -0.000487889f, -0.00162015f, -0.0268957f, 0.0158162f, 0.0124589f, 0.0514896f, 0.0391116f, -0.02102f, 0.0289451f, -0.0162062f, 0.0295524f, 0.0240599f, 0.00653552f, -0.0296798f, -0.0614426f, 0.00678693f, -0.0126935f, -0.0259306f, -0.0270236f, -0.005202f, -0.027559f, -0.00571665f, 0.01303f, -0.0176816f, 0.00828625f, -0.0159388f, 0.016197f, -0.0685197f, 0.0359586f, -0.0149305f, -0.0100357f, -0.054005f, 0.0405895f, -0.0436483f, -0.0196033f, 0.0205626f, 0.0601753f, 0.00745636f, 0.00526461f, 0.00770411f, -0.00536197f, -0.0196271f, -0.00742883f, 0.0673765f, 0.0225239f, 0.0330661f, -0.0197954f, 0.0635232f, -0.00196483f, -0.0160432f, 0.0274051f, 0.0249642f, -0.0215083f, 0.00376016f, 0.0484418f, -0.0339058f, -0.00930553f, 0.000391001f, 0.0489547f, 0.00680175f, 0.0121302f, -0.0159317f, -0.00746274f, 0.00762586f, 0.0151285f, -0.00984925f, 0.00967698f, -0.063813f, -0.00191317f, -0.0225768f, -0.0460198f, 0.0129389f, 0.022693f, -0.0331679f, -0.0252172f, 0.0152612f, -0.0615063f, 0.00776267f, 0.0890267f, -0.0218608f, 0.0164835f, -0.048754f, 0.0158734f, 0.00247796f, -0.0340838f, 0.0199824f, 0.0422744f, 0.00495236f, 0.00733676f, -0.693422f, -0.057195f, -0.042145f, -0.0894016f, 0.00573138f, 0.00168211f, -0.00815092f, 0.1004f, -0.00830388f, 0.0212194f, 0.00796229f, 0.0182782f, -0.00677567f, -0.0025772f, -0.0141583f, -0.0503938f, 0.00933939f, -0.0440368f, -0.0650577f, -0.0133163f, -0.0150479f, -0.128004f, -0.025883f, -0.0142512f, 0.0267747f, 0.0603829f, 0.0616747f, 0.00518816f, 0.0353825f, -0.0136665f, -0.0116953f, -0.0117363f, -0.00988685f, 0.0161024f, -0.0164802f, 0.0120735f,
        0.0115264f, 0.00956785f, -0.0348965f, -0.0115787f, 0.0441999f, 0.0345045f, 0.0134386f, -0.0337335f, -0.00245127f, -0.0610053f, 0.0043896f, 0.0019506f, 0.013525f, -0.0545739f, 0.0306072f, 0.105704f, -0.0610636f, 0.0184838f, -0.0121108f, -0.00898275f, 0.0264786f, 0.0351719f, 0.00565877f, -0.00984551f, 0.0349376f, 0.0065558f, 0.000771663f, 0.000747164f, 0.00623147f, -0.0100182f, 0.0147877f, 0.027002f, -0.0082708f, -0.00312388f, -0.031057f, 0.0352335f, 0.0102762f, -0.136548f, -0.00137814f, -0.0245331f, 0.0302073f, -0.050357f, -0.0055813f, -0.0035066f, 0.0159663f, -0.00413293f, -0.0220518f, -0.0378098f, -0.000528503f, -0.00883574f, -0.0160642f, -0.0976056f, -0.00949359f, 0.0667935f, 0.0152671f, -0.00275173f, -0.00305567f, -0.00027522f, -0.0358676f, 0.0613587f, -0.0621408f, 0.0603126f, -0.00382261f, -0.0162797f, 0.0627967f, -0.0338104f, 0.019684f, 0.0723154f, 0.0405459f, 0.0150282f, 0.0116941f, 0.0159087f, 0.0423308f, 0.000188638f, -0.0151563f, 0.0213552f, 0.0260785f, -0.000634076f, -0.00666879f, -0.0143571f, -0.0154005f, 0.0452614f, -0.0241995f, 0.00760913f, 0.00565907f, -0.0146403f, -0.00882357f, 0.109466f, 0.000185842f, 0.0530813f, -0.0167083f, -0.0132453f, 0.00510363f, 0.000928611f, -0.0231941f, -0.00849421f, -0.0127253f, 0.0143131f, -0.104331f, 0.0150856f, -0.0115339f, -0.0400927f, -0.00650179f, 0.00782663f, -0.0161432f, 0.00612369f, -0.0368485f, 0.0320765f, -0.000285285f, -0.0252538f, 0.00567933f, -0.00326235f, -0.0118118f, -0.0067807f, -0.0626707f, 0.0314245f, -0.00367115f, 0.0034559f, 0.00094028f, 0.012767f, -0.0376215f, -0.0102952f, 0.0236869f, 0.00184345f, -0.0418395f, -0.0542331f, -0.00655869f, -0.00491183f, -0.0167015f, -0.0135059f, -0.0126727f, -0.0262544f, -0.0235505f, -0.00927455f, 0.044421f, 0.0340354f, 0.0544527f, 0.0133111f, 0.00308665f, 0.00078136f, -0.0023735f, -0.0141342f, 0.00124783f, -0.0175074f, 0.0506524f, 0.0344784f, 0.016513f, 0.00434411f, -0.0224391f, 0.0865785f, -0.00372209f, -0.0103298f, -0.00164323f, -0.0143697f, -0.0125625f, -0.00602005f, -0.00435671f, -0.0097799f, -0.00277924f, 0.0124438f, 0.00866435f, 0.00456806f, 0.032294f, 0.00501145f, 0.0381001f, 0.0142146f, -0.0373586f, -0.0278584f, -0.0268059f, -0.0109542f, 0.0129881f, -0.0289077f, -0.00849425f, 0.00391238f, 0.0105073f, 0.0449334f, 0.00855353f, 0.0402285f, -0.00646413f, -0.00671409f, 0.013527f, -0.0528845f, 0.0319318f, -0.0113917f, -0.0113392f, -0.000316065f, 0.0412851f, -0.0162739f, 0.0137208f, -0.0163712f, 0.0349673f, 0.00457418f, -0.0198638f, 0.0765183f, -0.001026f, 0.0113388f, 0.00846672f, 0.0122229f, -0.0401006f, -0.00219702f, 0.00703645f, 0.0321573f, 0.000362714f, -0.24312f, -0.014646f, -0.00614563f, 0.0187569f, -0.00394876f, 0.0243838f, -0.00188284f, 0.0050112f, 0.0221267f, -0.00302741f, 0.0435336f, -0.0226377f, 0.0262879f, 0.0155468f, 0.0279725f, -0.00188527f, -0.00564561f, -0.00020769f, 0.0150204f, 0.13116f, 0.021348f, 0.00731956f, -0.0343524f, 0.00212442f, 0.0352829f, 0.526485f, -0.00325235f, -0.00250349f, 0.0161844f, -0.0453576f, -0.0154224f, -0.0407768f, 0.0031079f, -0.00879997f, 0.00831367f, -0.0461003f, -0.0249753f, -0.0173187f, 0.0510597f, 0.0221946f, -0.0149577f, 0.000957178f, 0.0111411f, 0.00876051f, -0.0220329f, -0.0046637f, -0.020372f, 0.00369127f, 0.039286f, -0.00385722f, 0.0115072f, -0.00474474f, -0.0141273f, -0.19162f, -0.0187427f, -0.00145075f, -0.00458649f, -0.00136821f, 0.0037382f, 0.0102019f, -0.0101349f, -0.0303892f, -0.697959f, -0.00391341f, -0.00169856f, 0.0454146f, -0.0300301f, -0.0387779f, -0.0249505f, -0.0183996f, -0.00471838f, -0.00533851f, 0.000305908f, -0.00737827f, -0.0143906f, -0.0612462f, 0.0117793f, -0.0296389f, -0.0045701f, 0.0974987f, -0.0222056f, -0.00917552f, 0.00540695f, 0.376f, -0.0369584f, 0.0818413f, -0.0806179f, -0.0591828f, -0.0292424f, 0.0175326f, -0.0141385f, 0.01833f, 0.0209717f, -0.0198613f, -0.0303378f, -0.00184021f, -0.095508f, 0.00121903f, 0.00795399f, -0.0660669f, -0.000692821f, 0.00370955f, 0.140168f, -0.000690335f, 0.0085036f, -0.0224978f, 0.0989872f, -0.103726f, -0.00133824f, 0.00176511f, 0.0226218f, 0.00723803f, -0.0136401f, 0.0136266f, 0.00908615f, -0.0421018f, -0.0535609f, -0.0230947f, -0.0338358f, -0.00108633f, -0.0356084f, -0.109221f, -0.014515f, 0.0077523f, 0.0139792f, -0.0248496f, -0.023008f, -0.0472426f, 0.0865438f, 0.000595621f, -0.0451802f, -0.0395005f, 0.0493621f, -0.00124904f, 0.0988936f, 0.0572095f, -0.0729679f, -0.00415711f, 0.161504f, -0.00328739f, -0.0133308f, 0.00799106f, -0.0163052f, -0.0209516f, 0.00308542f, -0.0129289f, -0.0510538f, -0.0122714f, -0.0362058f, 0.0683402f, -0.0126313f, 0.0263825f, 0.0168551f, 0.00470125f, 0.0204198f, 0.0145374f, -0.021401f, 0.00460656f, 0.085484f, 0.0781075f, 0.0251125f, 0.00791536f, -0.0189591f, -0.0431845f, 0.051558f, 0.017842f, 0.36608f, -0.0343333f, -0.0303445f, -0.0115494f, 0.0530173f, 0.0165506f, -0.0235855f, -0.052452f, -0.00888096f, 0.0221193f, 0.0386185f, 0.0353902f, 0.0246971f, -0.0122489f, 0.0512722f, 0.00400143f, 0.0255521f, 0.00548785f, 0.00233302f, -0.0253462f, -0.0966852f, 0.00378993f, 0.00350757f, -0.0310213f, -0.0279353f, -0.00233223f, -0.0220107f, 0.00163079f, -0.00717164f, 0.00659987f, -0.00608499f, -0.02305f, 0.00402512f, -0.32546f, 0.0706807f, 0.0274278f, 0.0267394f, -0.00604822f, 0.0361692f, -0.0515999f, 0.0369351f, 0.0124044f, 0.0716815f, 0.0053833f, 0.00673388f, 0.0250085f, -0.000686182f, -0.00550432f, -0.00231397f, 0.00181825f, 0.022164f, 0.0330005f, -0.00140523f, 0.0463948f, -0.0278037f, -0.0318544f, 0.0275073f, 0.0620945f, -0.0128747f, 0.0329174f, 0.0206743f, -0.0352932f, -0.00835452f, 0.0248623f, 0.119621f, -0.0292978f, -0.0132096f, -0.0302576f, -0.0178306f, 0.0209123f, 0.0229405f, -0.0236861f, 0.00108116f, -0.0799521f, 0.00532662f, 0.0127616f, -0.00190055f, 0.00847102f, 0.00451121f, -0.0637118f, -0.0302129f, 0.0119081f, -0.117328f, -0.00946109f, 0.0605782f, -0.0390624f, 0.0192556f, -0.0170363f, 0.0300991f, 0.0444662f, 0.0422317f, 0.0170539f, 0.0504948f, 0.0270332f, 0.00916911f, 0.0242343f, 0.00898315f, -0.0158267f, -0.0475899f, 0.0175909f, -0.000817633f, -0.0176624f, 0.0975135f, -0.00854145f, 0.0155055f, 0.00762038f, 0.0229743f, -0.0525053f, -0.0149161f, -0.0367894f, -0.104801f, 0.013039f, -0.120883f, -0.0715135f, -0.0193206f, 0.0158965f, -0.0748989f, -0.120509f, -0.0506567f, 0.0147239f, 0.107749f, 0.0659703f, 0.0220761f, 0.0242295f, 0.0180054f, -0.0111281f, -0.0171504f, -0.014431f, 0.083154f, 0.0241038f, 0.0115941f,
        0.0112054f, -0.208447f, -0.0871743f, -0.0362684f, -0.0110118f, 0.068481f, 0.0322887f, -0.0375058f, -0.0130676f, -0.101841f, 0.0479009f, 0.0459907f, 0.00208143f, -0.0880017f, 0.0160549f, -0.0533964f, -0.0336657f, -0.000403741f, 0.0274574f, 0.00649047f, -0.0278283f, -0.0254132f, 0.0467184f, -0.0375531f, 0.127941f, 0.0291329f, 0.00155753f, 0.00199031f, 0.0183402f, 0.155697f, 0.0500429f, 0.00407514f, 0.0229933f, -0.00482785f, -0.0220735f, 0.0390895f, -0.0863406f, -0.132777f, 0.00204372f, -0.0069423f, 0.0260759f, -0.031759f, -0.00107891f, -0.0218382f, 0.00464639f, -0.00370248f, 0.00721869f, -0.0152541f, -0.00113688f, -0.00731756f, -0.0459436f, -0.0122795f, -0.0212339f, 0.072953f, 0.0268922f, -0.00254329f, -0.00535364f, 0.0200235f, -0.019393f, 0.00740422f, -0.0515143f, 0.0410708f, -0.00789718f, -0.0633389f, 0.0544137f, -0.0580859f, 0.0325159f, -0.015541f, 0.0178216f, 0.289658f, -0.0234133f, -0.0074536f, 0.0255261f, 0.00291012f, -0.0219596f, 0.0246941f, -0.00560577f, 0.00899517f, 0.00914874f, -0.0254892f, -0.0521876f, 0.0629406f, -0.00645591f, 0.111561f, 0.0122516f, -0.0106223f, -0.0132192f, -0.0819937f, 0.0132221f, -0.00695472f, -0.0207924f, -0.0723628f, 0.0495887f, -0.0359372f, -0.04756f, -0.0288064f, -0.08486f, 0.285901f, -0.0527237f, 0.0401743f, 0.00317573f, -0.00912604f, -0.00509804f, -0.019646f, -0.0133663f, 0.00250147f, 0.00489291f, 0.017901f, 0.117288f, -0.0253837f, 0.0201622f, -0.0127631f, -0.000326688f, -0.0153231f, -0.0756543f, 0.113002f, -0.0181392f, -0.00927301f, 0.0726324f, 0.00722584f, -0.0730271f, 0.0245927f, -0.102462f, 0.0356965f, -0.0606429f, -0.0444952f, -0.0166311f, 0.00795211f, -0.00189904f, -0.0158499f, -0.0204771f, -0.0472794f, -0.0079858f, -0.0501545f, 0.102751f, 0.0584957f, 0.0372233f, 0.00862791f, 0.00449617f, -0.0237138f, 0.00679621f, -0.0152089f, -0.00387291f, -0.126512f, -0.0284672f, -0.0684034f, 0.0303137f, -0.0162955f, -0.0581197f, -0.220276f, -0.00417518f, -0.0689113f, -0.017655f, -0.0224894f, 0.0357768f, 0.0133865f, 0.022937f, 0.0472434f, -0.00953042f, -0.0159915f, 0.00998823f, 0.00600883f, 0.0533401f, 0.194183f, 0.477756f, 0.0191196f, 0.0227464f, -0.00284643f, -0.13471f, 0.0769816f, 0.01241f, -0.0497929f, -0.0935632f, 0.0292851f, 0.0178327f, 0.104592f, -0.0467304f, -0.00100124f, -0.0401962f, -0.0224538f, -0.00678469f, -0.073481f, 0.227438f, -0.00830996f, 0.073789f, -0.0239749f, 0.154952f, -0.0544236f, 0.0156297f, 0.19281f, 0.0326588f, -0.00926173f, -0.0288493f, 0.0228173f, 0.0186095f, 0.0415022f, 0.0290895f, -0.00247426f, -0.0898812f, 0.0274265f, 0.0393059f, 0.0222607f, 0.019877f, -0.150684f, -0.262853f, -0.0894445f, -0.0205114f, -0.00142168f, 0.126473f, -3.85201e-05f, 0.0356633f, 0.0269576f, 0.0157574f, -0.0432543f, 0.0279592f, 0.024804f, -0.0267448f, 0.0191669f, -0.0040675f, 0.0139007f, 0.00963236f, -0.0110146f, 0.137714f, 0.0166686f, 0.0200946f, -0.0611695f, -0.0639973f, 0.0055134f, 0.042783f, 0.0271225f, -0.0468356f, 0.0247138f, 0.0103724f, 0.00932251f, -0.0140851f, 0.0358128f, -0.0059887f, 0.0386251f, -0.00545864f, 0.0596616f, -0.0379678f, 0.0116168f, -0.0113317f, -0.0299328f, 0.0217457f, 0.0063076f, -0.00526829f, -0.012835f, 0.0163333f, -0.0390477f, 0.0108823f, 0.127479f, -0.00949771f, 0.000669599f, -0.00832522f, -0.00771118f, -0.554012f, -0.259737f, 0.00827122f, -0.000538992f, 0.0152035f, 0.05717f, 0.00494831f, -0.0414577f, 0.0166355f, 0.0400496f, -0.0114314f, -0.0214246f, 0.00867137f, -0.0404191f, -0.0166356f, 0.0428265f, 0.0146152f, 0.00234592f, -0.0864799f, 0.0226774f, 0.00508847f, 0.0203778f, -0.0583453f, -0.00666855f, -0.127756f, -0.00862127f, 0.0452925f, -0.0831513f, -0.00326817f, 0.00995622f, 0.116901f, -0.0877858f, 0.112396f, -0.102312f, -0.105516f, -0.0259396f, 0.00757632f, 0.00122858f, 0.0103624f, 0.0457345f, -0.0242102f, -0.0583132f, -0.012498f, -0.313943f, 0.0069556f, -0.0319396f, 0.0172862f, -0.00853725f, 0.0116005f, 0.125311f, 0.00419865f, 0.0476964f, 0.00896339f, 0.00977134f, -0.0925261f, 0.0156905f, -0.018496f, 0.0196972f, 0.0157389f, -0.00196949f, 0.0145061f, -0.0606428f, -0.0694258f, 0.0709404f, 0.00871243f, 0.00455373f, -0.00558034f, -0.0824924f, 0.0011513f, 0.0384797f, 0.00638306f, 0.00363507f, -0.0606946f, -0.0774373f, -0.020545f, 0.0937525f, -0.00557294f, -0.0987101f, -0.0864387f, -0.0108511f, -0.0149365f, 0.0481765f, 0.036998f, -0.112909f, -0.00983293f, 0.135054f, 0.071086f, 0.019128f, 0.00687174f, -0.00651517f, -0.0349884f, -0.00583317f, -0.0110052f, 0.0398168f, -0.0141334f, -0.0344924f, -0.00134893f, -0.0270122f, -0.000114596f, 0.0220215f, -0.0321631f, 0.0329176f, 0.0261847f, 0.170964f, 0.0083325f, -0.0209986f, -0.00422142f, 0.00124639f, -0.000368193f, 0.00871341f, -0.0488562f, 0.0170233f, 0.0236273f, -0.0163899f, -0.120393f, -0.151225f, -0.00206636f, 0.105974f, -0.00312998f, 0.0290657f, -0.014112f, -0.0107348f, 0.032648f, 0.026346f, 0.0817057f, 0.0319139f, 0.00208954f, 0.0860523f, 0.0385837f, 0.115668f, 0.0399777f, 0.00339539f, -0.00545459f, 0.0598012f, 0.00298756f, 0.0148942f, -0.0227489f, -0.0139737f, -0.00473305f, -0.0326132f, -0.0229166f, 0.0207593f, 0.00277288f, -0.00719371f, -0.0202886f, -0.00475025f, 0.00617697f, 0.0162748f, 0.124789f, 0.0101917f, -0.0547861f, 0.0414249f, -0.0484098f, -0.0963767f, 0.0484124f, -0.00538394f, 0.00789277f, -0.0102249f, 0.104348f, 0.00192805f, 0.00647828f, -0.0461272f, -0.00422982f, 0.0325315f, 0.0211869f, 0.0120108f, 0.0362735f, -0.100353f, -0.106846f, 0.0453949f, 0.108593f, -0.00433587f, 0.0477131f, -0.011734f, -0.00221842f, -0.0186096f, 0.0176472f, 0.0535756f, -0.00753715f, -0.00288954f, 0.0351324f, -0.000401414f, 0.0260439f, 0.0353749f, -0.0214858f, 0.000924544f, 0.000524206f, 0.0411247f, -0.0106592f, 0.0429043f, 0.0430829f, 0.029413f, -0.00455873f, -0.0157798f, 0.0105745f, -0.10904f, -0.0209529f, -0.0605732f, 0.0151213f, 0.0293344f, -0.030329f, 0.0388919f, 0.0830521f, -0.0636824f, 0.0834155f, -0.0213396f, -0.0029129f, 0.0130443f, 0.0276463f, 0.0168069f, 0.0131674f, 0.00569299f, 0.0319241f, -0.215148f, -0.080939f, 0.033579f, -0.0317194f, 0.0329596f, -0.000564258f, 0.0486169f, -0.0763688f, -0.0114993f, -0.0945774f, -0.0518434f, 0.0208386f, -0.059539f, -0.109562f, 0.0544319f, -0.0264226f, -0.090276f, -0.0850728f, -0.0434345f, -0.00627017f, 0.0531473f, 0.214103f, -0.0206121f, 0.0996398f, 0.155764f, -0.00199676f, -0.0115997f, -0.0355156f, 0.113538f, 0.0365645f, 0.0733962f,
        -0.00286558f, -0.000197294f, -0.00342685f, 0.0153883f, -0.0289065f, -0.00108885f, 0.0387835f, 0.00578341f, -0.0262422f, 0.0582573f, -0.0156079f, 0.00150583f, -0.0362458f, 0.0104373f, 0.0113533f, -0.0386981f, -0.00206408f, 0.00844815f, -0.053396f, -0.00516133f, -0.0270117f, -0.27301f, 0.0846544f, -0.00616813f, 0.0871727f, 0.00543487f, 0.00184716f, 0.000983837f, 0.0179262f, 0.0818094f, 0.00281717f, 0.000740774f, -0.19657f, -0.00320105f, -0.00607788f, 0.0319331f, -0.0235907f, -0.0270345f, -0.00849474f, -0.0110374f, -0.0746362f, -0.00860617f, -0.0237307f, -0.00439848f, 0.0264778f, -0.00958103f, 0.019552f, -0.00263648f, 0.0296515f, -0.00113696f, -0.124069f, -0.0114847f, 0.00308178f, 0.0198766f, 0.0900992f, 0.00727182f, -0.0940811f, 0.0216224f, -0.0459814f, 0.017883f, -0.0572063f, 0.0627439f, -0.00563362f, -0.0648773f, -0.00153448f, -0.0913932f, 0.0230685f, 0.00374598f, 0.034431f, 0.112274f, -0.326763f, 0.0153011f, 0.00631464f, 0.00570424f, -0.0595568f, -0.0114755f, -0.0054627f, -0.000223818f, -0.00287856f, -0.0252934f, -0.0108832f, 0.0186247f, -0.0140906f, -0.0239924f, -0.0332168f, -0.00818134f, -0.0261136f, 0.0112761f, -0.0570478f, -0.0484226f, -0.00280576f, -0.140804f, -0.0192205f, 0.0193394f, 0.0043392f, -0.0096851f, -0.0238295f, 0.0496011f, -0.00870349f, 0.0271804f, 0.00735628f, 0.00931979f, -0.000362012f, -0.00038859f, 0.098518f, -0.0510564f, -0.00233872f, 0.00517725f, 0.0101231f, -0.0331861f, 0.0441328f, -0.00924161f, -0.0059294f, -0.0159056f, -0.0810096f, 0.203707f, 0.00935022f, 0.00920423f, 0.0427866f, 0.0270535f, -0.0613705f, 0.0281747f, -0.0292151f, -0.011845f, -0.560809f, -0.0430764f, 0.0249193f, 0.001065f, 0.0495798f, -0.00604974f, -0.0115863f, -0.0841969f, -0.0400231f, -0.0234006f, -0.099013f, 0.0434646f, 0.000907694f, 0.0125445f, 0.0118042f, -0.00467421f, 0.00886041f, 0.0296945f, 0.0324396f, -0.0114072f, 0.0988003f, 0.00847453f, 0.0464346f, -0.00464305f, -0.0289332f, 0.0590643f, -0.0350208f, -0.0899201f, -0.0159029f, -0.0648027f, 0.00696909f, -0.00101221f, 0.0140877f, -0.0855302f, -0.000846032f, -0.0256277f, 0.00854884f, -0.00292961f, 0.209544f, 0.0872828f, 0.0246488f, 0.0291603f, -0.0784974f, 0.00920441f, -0.011242f, -0.0297102f, 0.0152799f, -0.0428288f, -0.0651387f, 0.0138869f, 0.0139815f, 0.0836656f, 0.0361113f, -0.0635471f, -0.0160178f, -0.0220017f, 0.234027f, -0.0400384f, 0.186927f, -0.0295061f, 0.00130944f, -0.0287178f, -0.0214042f, -0.0285818f, 0.0222618f, -0.00368823f, 0.0601194f, -0.0188088f, -0.0146725f, 0.0157483f, 0.21603f, 0.056817f, -0.20685f, -0.0254415f, 0.00525571f, 0.00219887f, 0.0530388f, 0.0607272f, 0.0061378f, -0.113869f, -0.16334f, -0.0464161f, -0.00694523f, -0.00488537f, 0.0286918f, 0.00290496f, 0.178755f, 0.0109929f, 0.110835f, -0.0642967f, -0.0333608f, 0.00169389f, -0.00546941f, 0.00973807f, -0.00576067f, -0.0205257f, 0.0511577f, -0.0266243f, 0.109812f, 0.0471989f, 0.0996845f, 0.0135877f, -0.0794984f, 0.0649346f, 0.0303168f, -0.0011697f, 0.00521801f, 0.0626395f, -0.00297682f, 0.0266726f, -0.000223535f, 0.0116355f, -0.0108245f, 0.000611158f, 0.00728507f, 0.0239288f, -0.00188282f, 0.0150957f, -0.040548f, -0.0589448f, 0.0328252f, -0.0915972f, 0.0805046f, -0.00811939f, 0.0772469f, -0.0716012f, 0.000604462f, 0.047583f, 0.0334997f, -0.000381467f, -0.00726828f, 0.00027943f, -0.0427843f, -0.0568598f, 0.0147649f, -0.00348073f, 0.00288838f, 0.00979242f, -0.00538436f, -0.024106f, 0.00541673f, 0.00529046f, -0.00278852f, -0.0222607f, -0.00626747f, 0.0973789f, -0.0795939f, 0.105127f, -0.337742f, 0.0172115f, 0.00255328f, -0.0330435f, 0.0063678f, 0.0471297f, -0.050865f, -0.00217128f, 0.0139913f, -0.00278459f, 0.0452206f, -0.0122722f, 0.00537665f, 0.0068003f, -0.0241691f, -0.00537261f, 0.00198657f, 0.0288662f, -0.0673232f, -0.00391073f, 0.0160158f, -0.0148616f, 0.00889894f, 0.0278599f, -0.0259723f, -0.0464762f, -0.0699778f, 0.0855682f, -0.00447207f, -0.105144f, -0.000995281f, -0.0146742f, -0.49647f, 0.0685417f, -0.000740646f, 0.0278313f, -0.00761982f, 0.0475931f, -0.0645097f, 0.119236f, -0.0570179f, 0.00915969f, 0.0156965f, 0.101129f, -0.0274397f, 0.0317f, 0.435965f, 0.0895423f, 0.0228896f, 0.0537683f, -0.0312062f, -0.0316729f, 0.00405423f, -0.00417011f, 0.053186f, 0.0124111f, -0.0636419f, -0.059223f, 0.00212677f, -0.00180764f, -0.0184438f, -0.00539991f, -0.0216965f, -0.0297828f, -0.00665945f, 0.0659594f, 0.109878f, -0.0859683f, -0.0195527f, 0.0856906f, 0.113261f, 0.0901811f, 0.00573377f, 0.0357797f, -0.0261576f, 0.0127095f, 0.00452054f, 0.0160191f, 0.0674667f, -0.0187489f, 0.00896214f, -0.00895184f, 0.388793f, 0.0155203f, -0.206128f, -0.0134212f, 0.0159576f, 0.240592f, -0.0244503f, 0.0595618f, 0.0056212f, -0.0505254f, 0.160077f, 0.0021605f, 0.111341f, -0.664956f, 0.031356f, -0.00658282f, -0.431486f, -0.0241319f, -0.437714f, 0.0186697f, 0.0143805f, -0.0139802f, -0.00777148f, 0.0223012f, -0.0458929f, 0.0103136f, 0.0203269f, -0.0121667f, -0.00358236f, -0.0347832f, 0.0310102f, 0.0940264f, 0.0402878f, 0.0779475f, 0.085935f, 0.0506573f, 0.0125433f, 0.00945608f, 0.00711064f, -0.0157027f, -0.00267093f, -0.0460969f, 0.00133153f, 0.0510218f, 0.0568231f, 0.00654478f, -0.0148599f, -0.00556127f, 0.0984337f, 0.0012008f, 0.0401073f, -0.00218267f, -0.0913605f, 0.0250143f, 0.0269926f, -0.00189873f, 0.145338f, -0.0106285f, 0.128684f, 0.0182833f, -0.0104387f, 0.058272f, 0.054818f, -0.0204594f, 0.0514151f, -0.0114196f, 0.0121938f, -0.0135972f, 0.00423344f, 0.0268584f, -0.0233103f, 0.0149913f, 0.00556167f, 0.175006f, 0.0460865f, -0.0531133f, -0.00530817f, 0.00775018f, -0.00568381f, 0.00309299f, 0.00404426f, 0.0611169f, 0.04162f, 0.0620172f, 0.0113454f, 0.0556293f, -0.000326539f, -0.0136839f, -0.00373327f, 0.0962103f, -0.0169842f, 0.0247842f, 0.0442757f, 0.0244144f, -0.0176649f, -0.00554654f, -0.0050203f, -0.0177601f, -0.02368f, 0.0243078f, -0.0571087f, 0.0184628f, -0.0841841f, 0.0331607f, 0.0279732f, -0.0822138f, 0.0293232f, -0.0722001f, 0.0163439f, 0.0191851f, 0.414194f, 0.456304f, 0.097353f, 0.033467f, -0.010367f, -0.00362604f, -0.00940526f, 0.0541993f, -0.0126803f, -0.0284043f, -0.126488f, 0.0276941f, -0.0072592f, -0.0112239f, 0.200614f, -0.0674165f, 0.0152713f, -0.0543701f, -0.0742834f, -0.0453187f, -0.0254072f, -0.0692672f, 0.0332971f, -0.0228297f, -0.000965714f, 0.0732683f, 0.0640799f, 0.00158938f, 0.047803f, -0.00266977f, -0.0100275f, -0.00643167f, -0.0383495f, -0.00409583f, 0.0385844f, 0.0659188f,
        0.0063133f, -0.00408226f, 0.121465f, 0.0301708f, -0.0181853f, 0.0601681f, 0.00325393f, 0.10642f, -0.0275263f, -0.0194839f, -0.0252979f, 0.0217105f, 0.0386137f, 0.0112424f, 0.0430641f, 0.0730034f, 0.0354242f, 0.013652f, -0.0293887f, 0.142649f, -0.0690173f, -0.0961422f, 0.0442838f, 0.0452969f, 0.118274f, 0.0323701f, 0.0187156f, 0.5255f, 0.0118736f, 0.225357f, -0.0130602f, -0.0104742f, -0.07411f, -0.114514f, -0.0436895f, 0.00986579f, -0.0838205f, -0.101698f, -0.00483559f, -0.00391671f, -0.0699783f, -0.0195803f, 0.0459022f, -0.0091508f, 0.0073998f, -0.0577818f, 0.0674949f, 0.0137614f, 0.0715333f, 0.00271481f, -0.00891188f, -0.0212177f, 0.0437716f, 0.0257086f, 0.0345469f, -0.180349f, -0.0603965f, -0.147289f, -0.00330522f, 0.0067096f, -0.0179399f, 0.0182082f, -0.0270762f, 0.0402878f, -0.0166916f, -0.0948335f, 0.029574f, 0.0969981f, 0.0529901f, 0.00293059f, -0.154666f, 0.0407095f, 0.0316545f, -0.0062415f, -0.0351574f, -0.0147547f, -0.0135113f, 0.00357694f, 0.0517612f, -0.101499f, -0.00291564f, -0.0056001f, -0.00857672f, -0.0101505f, -0.0323477f, -0.0263152f, -0.0116552f, 0.0247082f, 0.0227123f, -0.10951f, -0.0328793f, 0.411161f, -0.0130315f, -0.0227835f, 0.0106074f, -0.00307627f, 0.00495261f, 0.0545998f, 0.000595861f, -0.0242671f, 0.0299187f, 0.00166324f, -0.00666328f, -0.0078437f, 0.0280452f, -0.16448f, -0.0143541f, 0.026909f, -0.193269f, -0.0355148f, 0.0118665f, -0.0365043f, -0.00810059f, -0.0352678f, -0.0630561f, 0.0280126f, 0.30164f, 0.0875995f, 0.0694396f, 0.0103573f, -0.0283321f, -0.621525f, -0.0445668f, -0.0148087f, -0.313831f, -0.00408616f, 0.0349075f, 0.0231337f, 0.142115f, 0.00382164f, 0.0393434f, -0.108881f, -0.0101964f, -0.0303501f, -0.106503f, 0.0308691f, -0.0197364f, 0.0091609f, 0.00739707f, -0.021932f, 0.00100097f, 0.00910001f, -0.0272304f, 0.0244325f, -0.0534487f, -0.0124806f, 0.102616f, -0.0300018f, -0.0371498f, -0.0484335f, -0.0434477f, -0.0806446f, -0.0323094f, 0.0210301f, 0.016248f, 0.0884761f, 0.0521384f, -0.306267f, -0.0181587f, 0.0638134f, 0.00266205f, 0.0659853f, 0.0215718f, 0.030898f, -0.010891f, 0.0265176f, -0.0440084f, 0.0334551f, -0.0404191f, -0.05042f, 0.0401076f, 0.00569889f, 0.0642698f, 0.0118167f, -0.152626f, -0.0383063f, -0.241934f, -0.14967f, 0.000835922f, -0.0176463f, 0.00669299f, -0.100216f, 0.0636827f, -0.0246564f, 0.0233452f, 0.00916313f, -0.0360494f, -0.0143271f, 0.00748104f, 0.00808922f, 0.120031f, -0.0139543f, -0.0895863f, -0.0414794f, 0.143243f, -0.0137803f, 0.0207675f, -0.0347851f, 0.0721874f, -0.0414808f, -0.116213f, 0.00107106f, 0.0103554f, -0.13586f, -0.290486f, 0.00166402f, -0.015201f, -0.00145561f, -0.0154914f, 0.00163743f, 0.0822632f, 0.08017f, 0.0710966f, -0.013158f, -0.0632138f, -0.0111834f, -0.0178201f, 0.0112061f, -0.00430423f, -0.0674515f, 0.214633f, -0.00585192f, -0.0351569f, 0.375032f, 0.0448701f, 0.0256456f, 0.0743934f, 0.0211866f, -0.00896532f, -0.0415844f, 0.0122347f, 0.0118991f, -0.0877453f, 0.0304085f, -0.00665392f, -0.00567859f, -0.00832385f, 0.00138205f, 0.0402719f, -0.00329125f, -0.0122391f, 0.0130672f, -0.0699987f, -0.0336706f, 0.0130345f, -0.256598f, -0.00998923f, -0.0732391f, 0.16722f, -0.0470782f, 0.016357f, 0.0118742f, -0.0706653f, 0.00409f, -0.0124226f, 0.000505835f, -0.0507414f, 0.00258108f, 0.0198879f, 0.000320695f, 0.0112645f, 0.00723067f, -0.0107117f, -0.00964231f, 0.014985f, -0.000720747f, -0.00563631f, -0.128197f, -0.00191921f, 0.100766f, -0.0177464f, 0.0910596f, 0.132686f, 0.0851709f, 0.0140803f, -0.0459295f, 0.00891749f, 0.0917738f, -0.0520881f, -0.00429575f, -0.0104893f, -0.0285219f, 0.0370703f, -0.0241567f, 0.0214466f, 0.0260263f, 0.112436f, -0.0221967f, 0.003362f, 0.00552892f, -0.0382231f, 0.00763609f, 0.0270099f, -0.028698f, -0.00121651f, 0.000527033f, -0.0406943f, -0.0840261f, -0.00983556f, -0.0288269f, 0.00269151f, -0.136611f, 0.0220631f, -0.00476321f, 0.0281217f, 0.0243983f, -0.00436437f, 0.00491977f, 0.0540143f, 0.0410553f, -0.00945594f, -0.0711867f, -0.011407f, -0.0290617f, 0.0077444f, -0.0194761f, -0.0353022f, 0.0242323f, 0.121606f, 0.136937f, 0.117977f, 0.0648052f, 0.000369128f, -0.0286182f, -0.000851573f, -0.0675435f, 0.0374786f, 0.0108061f, -0.00134871f, -0.0419874f, 0.0271549f, -0.21822f, 0.268321f, -0.00535237f, 0.011111f, -0.0614932f, 0.0500974f, 0.0900748f, 0.0334851f, -0.101783f, -0.00498551f, -0.0075128f, 0.00031712f, 0.0485839f, 0.000919265f, 0.0326066f, -0.023036f, 0.0096988f, 0.0178391f, 0.0861196f, 0.0466213f, -0.0299909f, -0.0991148f, -0.0230341f, 0.334094f, -0.0382573f, 0.0395579f, -0.00590484f, 0.0206429f, 0.246985f, -0.0283786f, 0.0598143f, -0.0353774f, 0.091151f, 0.0944889f, 0.00249664f, 0.202462f, -0.00569812f, 0.00865333f, -0.00812537f, -0.188173f, -0.0627191f, -0.28001f, 0.00917071f, 0.0506412f, 0.0010405f, 0.0678395f, 0.16542f, -0.00219039f, 0.0110519f, -0.00379539f, 0.00535911f, -0.00791708f, -0.000717427f, -0.0325235f, 0.0842137f, -0.020968f, 0.192455f, 0.0856024f, 0.132173f, -0.00232728f, 0.0647325f, 0.104932f, -0.0235684f, 0.00335134f, 0.00515333f, 0.192284f, 0.0592319f, 0.143246f, -0.00214825f, -0.168829f, -0.0149753f, 0.00881463f, 0.00489184f, 0.0030815f, -0.0645487f, -0.236596f, 0.0211161f, 0.428909f, -0.0184283f, 0.150971f, -0.00403509f, 0.0892136f, 0.0527521f, -0.00892411f, 0.257531f, 0.0159127f, -0.0153799f, 0.0299046f, 0.00748111f, 0.02268f, -0.0283898f, -0.0224564f, -0.00329609f, -0.0642335f, 0.0385503f, 0.00387719f, -0.0795388f, 0.0385978f, 0.0338672f, -0.00181007f, 0.500546f, 0.0174027f, -0.00941603f, 0.00119533f, 0.161396f, 0.0277067f, -0.0113644f, 0.00243689f, 0.0240222f, 0.00074696f, -0.00329644f, 0.00571551f, 0.353842f, -0.0345694f, 0.0954816f, 0.022245f, 0.0639779f, -0.0209006f, -0.0100804f, -0.0223871f, 0.00248849f, -0.0231191f, -0.105286f, -0.0150994f, 0.00230265f, -0.0295301f, 0.0119341f, 0.00911531f, 0.0540066f, 0.0076047f, -0.0945892f, 0.0196067f, -0.0357786f, 0.0719775f, -0.0972845f, 0.142406f, -0.18177f, 0.00491428f, 0.000342362f, -0.0186926f, 0.0489506f, -0.0333847f, -0.017827f, -0.00585373f, 0.0250148f, -0.0496847f, 0.00595432f, 0.180951f, -0.0459607f, -0.0360709f, -0.168328f, -0.0724864f, -0.161582f, 0.0156965f, -0.0463856f, 0.00603378f, -0.0396591f, 0.100121f, 0.00849666f, 0.0438226f, 0.0247446f, 0.0309354f, -0.0876779f, -0.0223912f, 0.0149475f, -0.0619022f, -0.0198987f, 0.0258675f, 0.0760512f,
        0.0237833f, 0.00298876f, 0.0487694f, 0.00950606f, -0.074622f, 0.0192038f, -0.0202395f, 0.105125f, -0.0154085f, 0.0355691f, 0.00281225f, 0.00531638f, 0.0101454f, 0.0510713f, 0.0313131f, -3.24692e-05f, 0.0563302f, -0.00384794f, -0.0967057f, -0.00911184f, -0.034748f, -0.00885298f, -0.00145702f, 0.00841001f, -0.00386897f, 0.00954715f, 0.0060942f, -0.00779779f, 0.0341911f, 0.0373562f, 0.000677265f, -0.0620633f, 0.00208294f, -0.0215586f, -0.085074f, 0.0143441f, -0.0186877f, 0.00127867f, -0.01249f, -0.00504883f, -0.00104019f, 0.0121985f, 0.000512828f, -0.00772995f, 0.00468516f, -0.0139477f, -0.0211804f, 0.210879f, 0.00785329f, -0.000516933f, -0.00212956f, -0.0162727f, 0.00414868f, 0.0109553f, 0.000250999f, -0.00637749f, -0.00108913f, -0.00648906f, -0.0123977f, 0.0104616f, 0.0241319f, 0.0770632f, 0.00195405f, -0.00752428f, -0.0405081f, -0.0883033f, 0.0394711f, 0.0062544f, 0.0315002f, -0.0138193f, -0.0353362f, 0.00803457f, 0.0055575f, -0.00122304f, -0.00591179f, -0.000313378f, -0.00928775f, 0.00167335f, 0.00110711f, 0.0102733f, -0.0102128f, -0.0332447f, -0.0050578f, -0.0365285f, 0.00129188f, -0.00545454f, -0.0488076f, -0.0522689f, -0.0028496f, 0.0269232f, -0.00264586f, 0.00549725f, 0.0937312f, -0.0097157f, 0.000703438f, -0.0316939f, 0.00265145f, 0.00747435f, 0.00703635f, -0.0498706f, 0.0260258f, 0.00486406f, 0.00831138f, 0.00331964f, -0.0116462f, -0.000328743f, -0.0193854f, 0.012874f, -0.0140591f, 0.00294906f, 0.167637f, -0.00563081f, 0.00047881f, -0.0132155f, -0.088562f, -0.00763682f, 0.00861545f, 0.0484862f, 0.118604f, 0.00888342f, -0.0480975f, -0.0108402f, -0.00768345f, -0.214419f, -0.045855f, 0.000607434f, 0.00143275f, 0.000233664f, 0.00111974f, 0.0283561f, -0.0137152f, 0.035663f, -0.0231469f, 0.0205628f, 0.0685008f, 0.0106492f, 0.00590557f, -0.00685771f, 0.00424108f, 0.000113577f, 0.00595773f, 0.00665598f, 0.000441705f, -0.00402036f, -0.0262544f, 0.00611645f, 0.0116063f, -0.00424871f, 0.0342696f, 0.0381022f, -0.0588067f, -9.04306e-05f, 0.013434f, 0.0049054f, 0.0123942f, -0.000403249f, 0.0504587f, -0.00181204f, 0.00841684f, 0.0187689f, 0.0174106f, 0.00611652f, 0.00976013f, 0.000955711f, 0.00209072f, -0.0257193f, -0.0127599f, 0.00699173f, -0.0153516f, -0.00193625f, 0.0528177f, 0.0170662f, 0.0746572f, 0.00809554f, -0.027025f, -0.0257472f, -0.00256271f, -0.0890082f, -0.00221022f, -0.00891542f, -0.00903598f, -0.0144857f, 0.0554675f, -0.00986486f, 0.00189685f, 5.93501e-05f, 0.00462237f, 0.00532594f, 0.00433364f, -0.003124f, 0.04f, -0.000328486f, -0.0648411f, -0.00377033f, 0.139774f, 0.00230164f, 0.0115385f, 0.0125043f, 0.148022f, -0.0284796f, -0.00155402f, -0.00387695f, 0.00829478f, -0.0471497f, -0.0015643f, -0.00582674f, -0.00431319f, 0.000878919f, 0.00687072f, -0.00301133f, 0.00398096f, -0.00563914f, -0.0026393f, -0.00377055f, -0.0609272f, -0.118688f, 0.00517703f, 0.0836725f, -0.012182f, -0.0512972f, 0.0119928f, 0.0247734f, -0.0427426f, 0.0341825f, 0.0698612f, 0.00279914f, -0.00847926f, -0.0226391f, 0.020679f, -0.00144619f, -0.0104832f, 0.0195441f, 0.000150691f, 0.0815801f, -0.00616593f, 0.00379428f, -0.00447982f, 0.00261409f, 0.0600844f, -0.0213836f, -0.00804557f, 0.00325642f, 0.00854879f, -0.0814344f, -0.027769f, -0.00191851f, 0.00536533f, -0.0164033f, -0.00257131f, -0.00205376f, -0.0200541f, -0.0128954f, -0.00532982f, 0.0022407f, -0.00130887f, 0.00425618f, -0.00845818f, -0.00126148f, -0.0107566f, 0.00104842f, -0.00435674f, 0.00433842f, -0.0109865f, 0.000301519f, 0.00589863f, -0.00851759f, -0.00137109f, -0.0256632f, 0.0120122f, -0.00451766f, -0.0132172f, 0.0204377f, 0.00862719f, -0.00529603f, 0.0007616f, -0.00779072f, 0.000307369f, 0.0161384f, 0.0140168f, -0.00223271f, -0.0234216f, 0.00152691f, 0.00407567f, -0.00575267f, -0.0169706f, 0.00373715f, -0.0130443f, 0.0149063f, -0.00592504f, -0.00101738f, -0.00432452f, 0.00608682f, -0.00623923f, -0.0048846f, 0.00141049f, -0.00787022f, -0.00325903f, -0.00925192f, 4.10188e-05f, -0.00650579f, -0.00344007f, -0.00507379f, -0.010943f, 0.0033921f, 0.0262149f, -0.0109309f, -0.00218072f, 0.00487267f, -0.00424018f, 0.0190863f, -0.0205672f, -0.00521787f, -0.749656f, 0.0045255f, -0.0111087f, -0.00594957f, -0.00784532f, -0.00218566f, -0.00261733f, 0.00115839f, 0.00810127f, -0.00685174f, -0.000515265f, 0.00996413f, 0.00908507f, -0.010911f, 0.0199673f, 0.00424915f, -0.0168506f, -0.0127626f, -0.0068238f, 0.0141051f, -0.0106615f, 0.00332799f, 0.00636155f, -0.0260333f, 0.00595097f, 0.0191085f, -0.0049198f, 0.00793315f, -0.00309666f, 0.0137166f, -0.00473366f, 0.0127659f, 0.000838826f, 0.0352708f, -0.00566433f, 0.00439918f, 0.00403144f, -0.0103773f, 0.000578005f, -0.00181792f, -0.0300049f, -0.00661571f, 0.0085107f, 0.00894339f, 0.00861617f, 0.00351911f, 0.016009f, -0.00165849f, 0.00140448f, 0.00854556f, -0.000467159f, 0.00526625f, 0.0113457f, -0.000892589f, -0.00943319f, 0.016298f, 0.0129145f, 0.00977724f, -0.00864554f, -0.0149309f, 0.0109739f, 0.00925517f, 0.00301191f, -0.00253138f, -0.0198261f, 0.00383641f, 0.00511284f, -0.0561408f, -0.0281949f, -0.00444545f, -0.00338158f, -0.00161292f, -0.00978353f, 0.00446439f, 0.000485823f, 0.000591379f, 0.00729576f, -0.024535f, 0.00937071f, 0.00193014f, 0.00812366f, -0.015649f, -0.00101637f, 0.0112705f, 0.00182169f, -0.00906464f, 0.0080621f, -0.0130414f, -0.000293886f, -0.00548405f, -0.00557287f, -0.00444211f, 0.000131822f, -0.0116247f, 0.00918694f, 0.00706824f, -0.00459982f, -0.00134241f, 0.00769962f, -0.000905408f, -0.00643464f, 0.00195699f, 0.0103661f, 0.0117231f, 0.00141366f, 0.013737f, -0.00475491f, -0.00389627f, -0.008428f, -0.00336822f, -0.0123985f, -0.00384732f, -0.00772105f, -0.00399041f, 0.00441658f, -0.0179348f, 0.00088589f, 0.00130237f, -0.00910743f, -0.000932973f, -0.000705488f, -0.00845157f, -0.00409019f, -0.00198943f, -0.00037801f, -0.0110968f, -0.00639611f, 0.00967489f, -0.00286205f, -0.00142743f, 0.00952024f, 0.0067011f, -0.00771389f, 0.000101275f, 0.00173372f, 0.000959312f, 0.00841471f, 0.00336334f, 0.00371336f, 0.00482025f, -0.00711383f, 0.00583148f, 0.0108545f, -0.000470039f, -0.0110626f, 0.00324574f, 0.025979f, 0.0153801f, -0.00239289f, -0.0364105f, -0.0252222f, 0.00766028f, -0.000371992f, -0.00263989f, 0.0215774f, 0.0230998f, -0.00223724f, -0.000281751f, -0.00482297f, -0.0175295f, -0.00712851f, 0.0106509f, 0.00430235f, 0.00410187f, 0.00823292f, 0.00280169f, 8.28998e-05f, -0.00169138f, -0.00976853f, -0.00530213f, -0.00814388f, 0.0013187f, 0.00816157f, 0.00138731f, -2.68979e-05f, -0.0103893f, -0.0500543f, 0.000847671f, 0.00327953f, 0.00418289f, 0.0180997f, -0.00027566f, -0.00544788f, -0.0076323f, -0.00551657f, -0.00599236f, -0.0127374f, -0.0174632f,
        -0.000449777f, -0.000137405f, -0.0762075f, 0.000949166f, 0.0346124f, -0.0111424f, 0.0108357f, 0.0121679f, 0.0242749f, 0.052692f, -0.0017713f, 0.0053728f, 0.0128862f, -0.0162366f, 0.0125041f, -0.00602398f, 0.0107778f, -0.00323086f, -0.00914208f, -0.013884f, 0.00755173f, -0.0175622f, 0.00473339f, -0.015003f, -0.0238219f, 0.004502f, 0.00187154f, 0.0041163f, -9.36184e-05f, 0.00873372f, 0.0121869f, -0.020973f, -0.006006f, -0.0038208f, 0.00210471f, 0.00255549f, -0.0251856f, -0.0626372f, -0.0059258f, -0.0058662f, -0.0946306f, 0.00197436f, 0.00105865f, -0.0033595f, 0.0158977f, -0.0036025f, -0.00568902f, -0.0202577f, -0.000251319f, -0.0117895f, -0.0144239f, -0.0144024f, -0.0150431f, -0.0354826f, -0.0135123f, -0.000422157f, 0.0286438f, -0.000884989f, -0.00675718f, 0.013241f, -0.0118388f, 0.0321394f, -0.000803071f, 0.11408f, -0.00806301f, -0.00831608f, 0.0165189f, 0.016094f, -0.000449332f, -0.00695901f, 0.0437514f, -0.00172117f, 0.00180391f, -0.000859933f, -0.0144826f, 0.0262613f, -0.00194352f, -1.98829e-05f, -0.00902827f, -0.00400867f, -0.00600827f, 0.0120846f, -0.0162493f, 0.0418596f, 0.00131911f, -0.00631566f, 0.00270484f, -0.0950513f, 0.00726431f, -0.0169798f, -0.000554365f, -0.00256903f, -0.00885843f, 0.0104025f, 0.00590779f, -0.00175832f, 0.0168603f, 0.00964353f, -0.0180614f, 0.0213157f, 0.0209548f, -0.0231143f, -0.00121617f, -0.0129815f, -0.0199287f, 0.00863336f, -0.00464991f, 0.0162288f, -0.340115f, -0.011018f, -0.0593997f, 0.00644821f, 0.0416332f, 0.0394596f, 0.0172296f, 0.00494231f, 0.0143805f, -0.00819845f, 0.00196982f, 0.00393258f, 0.0246168f, -0.0235927f, 0.0131416f, -0.0190432f, -0.0237865f, -0.0155627f, 0.0265165f, 0.0162884f, 0.00321098f, 0.0136674f, -0.000966112f, -0.0100813f, -0.00604589f, 0.00889466f, 0.0113945f, 0.0264707f, 0.00371883f, -0.00843358f, 0.0145675f, 0.0048638f, 0.00110399f, -0.00130233f, 0.00740726f, -0.00393368f, -0.0242178f, 0.00341681f, 0.00115369f, -0.00297881f, -0.0844071f, 0.0537151f, -0.00209399f, 0.0310295f, 0.0383914f, 0.00456459f, 0.0188114f, -0.0177144f, 0.0133258f, 0.0584683f, -0.00640495f, 0.0175946f, 0.0186782f, 0.00213311f, 0.00393403f, 0.00382759f, 0.00267507f, 0.00493673f, -0.00856695f, -0.00627955f, -0.0103436f, -0.000671664f, -0.110419f, 0.0307264f, 0.0042176f, 0.0031638f, 0.0154172f, 0.00265482f, 0.0410853f, 0.00833895f, -0.0183989f, -0.000717906f, -0.0090387f, -0.00404523f, -0.00976238f, -0.0137555f, 0.000157289f, -0.00341186f, -0.0214878f, 0.0142639f, 0.00624623f, 0.000537292f, -0.0520912f, -0.0432221f, -0.00330415f, 0.0263942f, -0.00150974f, 0.00172088f, -0.0815726f, -0.0201155f, -0.00986346f, 0.0121252f, 0.00198959f, -0.0349936f, -0.00608366f, -0.00399543f, 0.0192487f, -0.0123156f, 0.0072797f, 0.000507143f, 0.0334805f, 0.000609379f, 0.00961966f, -0.00697663f, 0.00201967f, -0.0207349f, -0.0103385f, -0.00343849f, -0.00330492f, 0.035106f, -0.00456996f, 0.00197528f, 0.016148f, 0.0142903f, 0.0616483f, 0.0093118f, -0.0596028f, 0.00945764f, -0.00659242f, 0.118389f, -0.00259384f, -0.00285344f, 0.00567036f, 0.0195813f, -0.00461807f, -0.0608699f, 0.00380259f, 0.00143385f, -0.00466997f, 0.0194046f, -0.0198423f, -0.00334569f, -0.014399f, 0.0130021f, -0.0141619f, -0.00859914f, 0.00997122f, -0.0198446f, -0.0094162f, -0.0116609f, -0.0111888f, -0.00903524f, 0.00937981f, 0.01772f, -0.00236374f, -0.00870162f, 0.000141193f, -0.0343695f, -0.00997931f, 0.0073531f, -0.100394f, -0.00367661f, -0.00124499f, 0.00318026f, 0.0554203f, -0.00342582f, -0.0104147f, -0.0577869f, -0.0126485f, -0.0332496f, 0.0346141f, 0.0307962f, -0.0174745f, -0.0387339f, 0.0167707f, -0.0363424f, 0.0154902f, -0.0118644f, -4.63543e-06f, -0.0683506f, -0.0344076f, -0.00104884f, -0.00883997f, -0.00305185f, -0.0150299f, -0.0186403f, 0.0110238f, 0.00779224f, -0.0102231f, 0.0087488f, -0.0138988f, -0.0229105f, -0.0244903f, -0.0202919f, 0.00135903f, -0.00574432f, 0.00254918f, 0.0340209f, -0.046428f, -0.00670622f, 0.000925543f, -0.0249251f, -0.00275456f, 0.0199177f, 0.000210993f, 0.027762f, -0.0228046f, 0.0484813f, 0.00538959f, 0.0136714f, -0.00690097f, -0.0448533f, -0.00815204f, 0.00734891f, 0.0173959f, -0.0379109f, 0.0594617f, -0.00722084f, 0.0415935f, 0.014792f, -0.0170252f, -0.0139396f, 0.00146415f, 0.00117702f, 0.0685559f, 0.00727832f, -0.107566f, -0.0112505f, 0.033853f, 0.0046957f, -0.0242369f, 0.0148181f, -0.0723487f, -0.00961667f, 0.0304085f, -0.00520772f, -0.0316467f, 0.0327801f, -0.00755137f, 0.0166041f, -0.0557288f, -0.0227759f, -0.00314548f, 0.0152585f, 0.020071f, -0.0377076f, 0.00687613f, -0.0273935f, -0.00647955f, 0.0105047f, -0.0137238f, 0.023264f, -0.0455722f, -0.00221414f, -0.0258535f, -0.0236395f, 0.0593407f, 0.00448763f, 0.0150777f, 0.00437925f, 0.0295782f, -0.0344752f, 0.00365267f, 0.140464f, -0.0479012f, 0.025726f, 0.119063f, 0.000301925f, -0.00810565f, -0.354073f, -0.0723185f, -0.0046123f, 0.033882f, -0.044552f, -0.0138361f, 0.00384129f, 0.0139111f, -0.01667f, -0.0821503f, 0.0029974f, -0.0306725f, 0.0160366f, 0.0334754f, 0.0192693f, -0.00616713f, -0.00232275f, 0.0107987f, 0.00437057f, 0.0017298f, 0.0196916f, -0.0417255f, -0.00911193f, 0.00876709f, -0.00172422f, -0.00105248f, -0.0191631f, -0.00387423f, -0.0102766f, -0.025317f, -0.0416204f, -0.0319611f, -0.00359193f, 0.00424064f, -0.00575092f, -0.0282402f, 0.0745899f, -0.0126492f, -0.0162564f, -0.261967f, -0.705265f, -0.0403731f, -0.00209634f, -0.694297f, 0.00956909f, 0.0158826f, 0.0130207f, 0.003825f, -0.000300812f, -0.0121346f, 0.00642053f, -0.012902f, 0.0309272f, 0.0609192f, -0.00654145f, -0.0937578f, -0.00432024f, -0.00767539f, 0.0461248f, 0.00701077f, -0.0174477f, 0.00563833f, -0.0107107f, -0.0255275f, 0.00892488f, -0.00166062f, 0.039829f, -0.00150394f, 0.00742194f, -0.00885529f, -0.0103532f, 0.0777858f, 0.0885367f, -0.00425715f, 0.0423651f, -0.0446651f, -0.635069f, -0.00919329f, -0.00356176f, 0.00988705f, 0.0116529f, -0.0401253f, 0.00260105f, 0.00573955f, -0.0667439f, 0.101175f, 0.0765288f, -0.0120077f, 0.00322599f, -0.0192768f, 0.0382749f, -0.222119f, -0.0452036f, 0.0424303f, 0.0890699f, 0.0117557f, 0.0315167f, 0.0284256f, 0.00541845f, -0.250147f, 0.00420668f, -0.0189724f, -0.00416381f, -0.00162803f, -0.0108763f, -0.00970892f, 0.0134476f, -0.0254931f, 0.0307225f, 0.00128596f, 0.0171106f, 0.00467854f, -0.0124376f, 0.0183396f, 0.0021754f, 0.00170886f, -0.0226898f, 0.0250111f, -0.0533301f, -0.0163268f, 0.00618995f, 0.0416378f, 0.0475397f, 0.0105684f, -0.00440933f, 0.0496722f, -0.0215733f, -0.0256361f, -0.0285091f, -0.0276881f, -0.00102202f, -0.0720219f, -0.0296656f,
        0.00465617f, 0.00138814f, -0.0913312f, -0.0161213f, 0.0160887f, 0.0204469f, -0.0223319f, 0.015304f, 0.000397867f, 0.00824013f, 0.0114613f, 0.00408309f, 0.0384456f, -0.00453968f, 0.0176576f, 0.100434f, -0.0393971f, 0.0160015f, -0.00313166f, -0.0058054f, 0.0342083f, 0.0333727f, 0.00275399f, -0.0111393f, -0.0656798f, 0.0117794f, 0.00399766f, 0.00310487f, 0.00290905f, 0.00311256f, 0.0103328f, 0.00221549f, -0.00340486f, -0.00955604f, -0.010614f, 0.0144013f, -0.0244803f, 0.246714f, 0.00585756f, -0.0183366f, 0.0131221f, -0.015529f, 0.0634503f, -0.00107566f, 0.0230663f, -0.00523926f, -0.0100486f, -0.0270644f, 0.0938544f, -0.0136558f, 0.0164469f, -0.349288f, 0.0108305f, 0.0621752f, -0.00813808f, -0.0218271f, 0.0168811f, -0.00509217f, -0.0249135f, 0.0268669f, -0.0294336f, 0.0396944f, -0.00419361f, 0.00843219f, -0.000475472f, -0.0122415f, 0.0142385f, 0.0240099f, -0.0041296f, 0.0167314f, -0.0210217f, -0.00275032f, 0.0121842f, -0.00556776f, -0.0215306f, 0.0411878f, -0.00102203f, 0.00011487f, -0.0142263f, -0.00257424f, -0.0044306f, 0.0115836f, -0.0331884f, 0.0153153f, 0.0023461f, -0.0229996f, -0.00982945f, 0.0207273f, 0.0039542f, -0.0275622f, -0.00118208f, -0.00703868f, -0.0111554f, 0.0155981f, -0.0197133f, -0.00157645f, 0.0790344f, 0.0277319f, -0.0239723f, 0.0133704f, 0.0153687f, -0.0220235f, -0.0652554f, 0.0340702f, -0.0256995f, 0.00463251f, -0.134567f, 0.0048301f, -0.0935251f, -0.0125128f, -0.0560035f, -0.000903825f, 0.0231884f, 0.0678238f, 0.0172834f, 0.0226948f, -0.00784814f, -0.000168366f, 0.0165854f, 0.00979108f, -0.010978f, -0.147669f, 0.020833f, -0.0320907f, -0.339001f, -0.0307849f, -0.00796792f, 0.00704321f, -0.0258511f, 0.0302859f, -0.0174755f, -0.0208662f, -0.00800382f, -0.00772683f, 0.00787931f, 0.0244046f, 0.0635711f, -0.0490687f, 0.00843431f, -0.00969577f, -0.00403176f, -0.00225678f, -0.00425568f, 0.00423476f, -0.0522863f, 0.00901175f, 0.00701737f, 0.0203201f, 0.00764967f, -0.0128627f, -0.0154611f, -0.00973917f, 0.0172989f, 0.00679487f, -0.00897315f, -0.00337138f, -0.0103584f, -0.00507785f, -0.00390477f, 0.0472275f, 0.0060846f, 0.0151745f, 0.0472687f, 0.000490868f, 0.0196255f, 0.00541134f, -0.0206129f, -0.00112977f, -0.0197924f, -0.0553976f, -0.098063f, 0.0664134f, 0.00349375f, 0.00311233f, 0.0401445f, 0.0128354f, -0.0250036f, 0.0436594f, -0.0462325f, -0.00102946f, -0.013474f, -0.0172785f, 0.0394013f, -0.00569089f, 0.000160535f, 0.000504291f, 0.0504433f, -0.0205918f, 0.0101148f, -0.00946464f, -0.0885629f, -0.04032f, -0.012075f, 0.492342f, -0.000999111f, 0.00407901f, 0.0888248f, 0.0100317f, -0.024372f, -0.0211601f, 0.000658811f, -0.0209988f, -0.0190039f, -0.0219266f, -0.0516314f, -0.00642571f, 0.00488745f, 0.00512097f, 0.0145898f, -0.00157307f, 0.0026168f, 0.0156606f, -0.00531944f, -0.017507f, -0.0180003f, 0.00282254f, 0.0143295f, 0.0777137f, -0.00385748f, -0.00549398f, -0.0172826f, 0.0323722f, 0.185825f, 0.0121615f, 0.00399867f, -0.0541097f, 0.0386216f, 0.0595922f, 0.594257f, -0.00955271f, 0.00343269f, 0.0139925f, 0.00328999f, -0.0792421f, -0.045498f, 0.0113837f, -0.00976291f, 0.00624078f, -0.0254107f, -0.0216194f, -0.028773f, 0.0236943f, 0.0197444f, -0.00939094f, 0.0135671f, -0.0407697f, 0.00794318f, -0.0184558f, -0.0282076f, -0.0112124f, 0.00710705f, 0.0203747f, -0.00201855f, -0.0137849f, -0.00224183f, -0.00758043f, 0.0109492f, 0.0111736f, -0.0524165f, -0.00359813f, -0.0105491f, 0.00795013f, 0.0490089f, -0.0172285f, -0.131601f, -0.640844f, -0.00210558f, -0.0191391f, 0.144537f, -0.0187546f, -0.0117677f, -0.0243942f, -0.0673674f, 0.0116665f, -0.00634048f, -0.0171121f, -0.018849f, -0.0452217f, -0.0314511f, 0.01823f, -0.0338747f, -0.00232084f, -0.0184449f, -0.0628265f, -0.00846206f, 0.00285066f, 0.281056f, -0.0109403f, -0.036282f, 0.00725135f, -0.027479f, -0.0120889f, 0.0185699f, -0.00228023f, 0.000971992f, 0.020036f, -0.0437852f, -0.013831f, 0.0284799f, -0.0116033f, -0.0213317f, -0.0391473f, -0.0180216f, 0.0224665f, 0.00661723f, 0.0188164f, -0.00856477f, -0.0188785f, -0.0419517f, -0.0383142f, 0.00822795f, -0.0210551f, 0.0376673f, -0.0158509f, 0.0531296f, -0.0222652f, 0.0202294f, 0.0377989f, -0.0486931f, -0.0236611f, -0.0364076f, -0.0364403f, 0.105507f, -0.0520728f, -0.085646f, -0.0517868f, 0.00898522f, 0.0145328f, -0.0152412f, 0.00230019f, -0.0490983f, 0.0199105f, 0.193699f, -0.00652485f, -0.0293521f, -0.101157f, 0.00759732f, 0.0611226f, 0.00668415f, -0.0644944f, -0.00138395f, -0.0872389f, -0.0289147f, -0.0104552f, 0.0102965f, -0.00918203f, -0.0163947f, 0.00688836f, -0.0460991f, 0.0010579f, -0.0220147f, 0.00389295f, -0.0450669f, -0.0338309f, -0.00643917f, -0.164896f, 0.00520622f, -0.00943891f, 0.015696f, -0.0488516f, 0.00357405f, 0.395393f, 0.0142406f, 0.0375136f, 0.0266987f, 0.00442581f, -0.0355697f, 0.0566785f, -0.0609618f, 0.0953531f, 0.0234361f, -0.0235014f, -0.0201052f, 0.0185904f, 0.0944014f, -0.00254259f, 0.0149094f, -0.00267577f, -0.0236442f, 0.0304207f, 0.0195184f, 0.00453831f, -0.010829f, -0.00384567f, -0.00720987f, 0.00142745f, 0.00339592f, 0.0255406f, -0.0328377f, -0.0418446f, 0.00524565f, -0.019943f, -0.00744414f, -0.0262656f, -0.00295384f, -0.012041f, 0.00168772f, -0.0393009f, -0.0333347f, -0.0127033f, -0.0399219f, -0.12722f, -0.223577f, 0.0811929f, -0.130626f, -0.0705225f, 0.174048f, 0.0435034f, -0.136602f, 0.00640297f, -0.166342f, 0.0597288f, 0.0182928f, 0.00638083f, 0.00566142f, 0.0143743f, -0.0117229f, -0.00092003f, -0.00302193f, 0.0193828f, 0.0549159f, -0.01403f, -0.0686686f, -0.00131562f, -0.0395576f, 0.0140634f, 0.00728921f, -0.0222314f, 0.0847774f, 0.00397858f, -0.037106f, 0.00703206f, 0.0217107f, 0.026982f, -0.0970178f, 0.00170535f, 0.00461989f, -0.0484043f, 0.0549405f, -0.00663961f, -0.0301618f, 0.0402775f, -0.126174f, 0.042974f, 0.00767555f, -0.0323881f, -0.0021808f, 0.00152122f, -0.0794255f, 0.00950137f, 0.00617034f, -0.186531f, 0.0667047f, 0.158624f, -0.0498641f, 0.000181888f, -0.00194408f, 0.0130678f, -0.0624929f, 0.099144f, 0.00810417f, 0.174436f, 0.0147924f, 0.00815054f, 0.0152255f, -0.0833151f, -0.072767f, -0.201512f, -0.0109339f, -0.003133f, -0.00430304f, -0.0208616f, -0.0187232f, 0.0277294f, -0.451013f, 0.0336152f, -0.00462652f, 0.00806012f, -0.000483294f, 0.0313363f, 0.0948398f, -0.0302999f, -0.00779582f, -0.0975373f, 0.0429978f, -0.0117262f, -0.00451523f, -0.0175741f, 0.0914118f, 0.0390275f, 0.00306197f, 0.0172763f, 0.0486995f, -0.0628708f, -0.00845093f, 0.00565009f, -0.0126375f, 0.0362389f, -0.0893211f, -0.0264466f,
        0.0309426f, -0.0247239f, -0.0618656f, -0.16444f, 0.0416493f, -0.0039234f, -0.0446445f, -0.0806408f, 0.0315374f, -0.0123988f, 0.0385759f, 0.0315165f, 0.00742563f, -0.0276244f, 0.013597f, -0.000546713f, -0.126003f, -0.0403999f, -0.0199147f, 0.090123f, 0.0122743f, 0.0904552f, 0.0480448f, -0.0274991f, -0.0463688f, 0.132874f, -0.0163207f, 0.00931698f, 0.00050237f, -0.034227f, 0.0273549f, 0.0257694f, 0.0545361f, -0.0196519f, -0.00616926f, 0.0252382f, 0.00394299f, 0.00503618f, -0.000107687f, -0.00739968f, 0.0155088f, -0.0271828f, 0.0136159f, -0.0184294f, 0.00419291f, -0.0705982f, 0.00832841f, -0.0455188f, 0.0203078f, -0.0104058f, -0.00448528f, 0.0346675f, 0.00227903f, 0.0283768f, 0.0146701f, 0.0238016f, -0.0041065f, -0.00951874f, -0.0656203f, 0.00289312f, -0.0280637f, 0.064775f, -0.0145084f, -0.0166982f, 0.112919f, -0.030709f, -0.08767f, 0.0231176f, -0.00683745f, 0.145201f, -0.0588483f, -0.00211676f, 0.0707442f, -0.0175353f, 0.0425204f, 0.047214f, -0.00454212f, 0.108341f, -0.0655429f, -0.0661698f, -0.00742549f, 0.0525604f, -0.00200138f, 0.0760939f, 0.0208251f, -0.0183413f, -0.019956f, 0.0497461f, -0.00312012f, -0.026077f, -0.00492334f, -0.0389153f, -0.0240003f, -0.0236527f, -0.00949685f, 0.00834218f, 0.196113f, -0.0203076f, -0.0373067f, 0.0511745f, -0.000502779f, -0.0506356f, 0.0270005f, 0.0560514f, -0.0566957f, 0.00592365f, -0.0950855f, 0.0330845f, 0.0126008f, -0.0178738f, 0.00655207f, -0.00560155f, 0.0226922f, 0.122885f, -0.0227311f, -0.0185407f, -0.024025f, 0.000734875f, -0.0501656f, 0.00259467f, -0.0401208f, -0.00270448f, 0.0298842f, -0.0449168f, -0.083653f, -0.0667249f, -0.012424f, 0.0228182f, -0.0256871f, 0.0103425f, 0.00584589f, -0.0313978f, -0.00512387f, -0.0389378f, 0.00783504f, 0.0246462f, 0.0204282f, -0.0313174f, 0.0293227f, -0.0135298f, 0.0250816f, 0.00154453f, 0.00455047f, 0.0664336f, -0.0924272f, 0.0141598f, 0.0249505f, 0.0114919f, 0.127537f, -0.0302333f, -0.0464173f, 0.0312457f, 0.0119746f, 0.00862732f, -0.0221585f, -0.00284848f, 0.014157f, 0.0253277f, 0.00495452f, 0.00886403f, 0.00389645f, -0.0347684f, -0.0039163f, 0.0218669f, -0.0417104f, 0.00547612f, -0.013528f, -0.00265715f, 0.180858f, -0.000752272f, -0.18944f, 0.0260848f, -0.000632882f, 0.0126054f, 0.0359676f, 0.0302849f, -0.0371376f, 0.0941217f, -0.0281283f, -0.0280773f, -0.011986f, -0.0406752f, 0.239648f, -0.00517518f, -0.00410975f, 0.00103368f, 0.0209206f, -0.0476301f, 0.00454544f, -0.0149667f, -0.0314583f, -0.00242636f, -0.0512553f, 0.0608112f, 0.0428258f, 0.0173526f, 0.0602241f, -0.0548611f, -0.131965f, -0.0495486f, 0.00765915f, -0.062264f, -0.000979455f, -0.0652348f, -0.147691f, -0.0231597f, 0.0251012f, -0.0946399f, 0.0277068f, -0.00621829f, 0.0313192f, 0.0259072f, 0.00394534f, -0.0118648f, 0.004981f, 0.0594206f, -0.0358001f, -0.0710233f, -0.00969833f, 0.023656f, -0.0388052f, -0.00855584f, 0.259141f, 0.0142973f, -0.00158563f, 0.0164536f, 0.0212657f, 0.00174633f, 0.0514006f, -0.00881672f, 0.0221807f, 0.0413859f, 0.0143335f, -0.163744f, 0.236609f, 0.0189168f, -0.0167902f, 0.0688642f, -0.0370002f, -0.0330411f, -0.0653769f, 0.00270779f, -0.00759605f, -0.0221796f, 0.0385442f, -0.0446415f, 0.06948f, -0.033133f, -0.0352207f, -0.0310347f, 0.00721417f, 0.0857527f, 0.00283876f, -0.115239f, 0.0347347f, -0.0365242f, 0.0587821f, 0.00664576f, -0.0273541f, -0.016766f, -0.0138301f, 0.00564337f, 0.0364023f, -0.0560315f, -0.0449002f, -0.0932135f, -0.0177926f, -0.0494535f, 0.0610707f, -0.00528969f, 0.114377f, -0.0275389f, 0.0177389f, -0.0280061f, -0.00589614f, -0.00858413f, 0.0105453f, -0.0247948f, -0.0472122f, -0.000931705f, -0.0574841f, -0.0412944f, 0.00216405f, -0.0681429f, -0.00229429f, 0.00222781f, -0.0102497f, -0.0110639f, 0.0254925f, 0.0135797f, -0.0289002f, 0.00603638f, 0.0356664f, -0.0870163f, 0.552476f, 0.0106117f, -0.025193f, -0.0567232f, 0.00731144f, -0.00597211f, 0.00564131f, -0.037914f, 0.00553956f, 0.0244306f, 0.0163081f, 0.0614898f, -0.0103462f, -0.0125773f, -0.0129543f, -0.0425792f, -0.00984468f, 0.0241087f, 0.0391885f, 0.0113726f, 0.0740247f, -0.0314575f, 0.0847706f, 0.00766129f, -0.00782563f, -0.00219977f, 0.0364213f, 0.00561357f, -0.0207095f, -0.0389947f, -0.0574235f, -0.0215928f, 0.0242519f, 0.0150763f, 0.00640004f, 0.0049859f, -0.0883498f, 0.0259088f, -0.00976872f, -0.0257561f, -0.145433f, 0.0186583f, -0.0313577f, 0.0232484f, 0.135472f, -0.0611472f, -0.0134871f, -0.0152308f, 0.0481365f, -0.000509527f, 0.0241717f, -0.0205968f, -0.0464828f, 0.00742741f, -0.0585818f, 0.0174123f, -0.032865f, 0.0399474f, 0.0189778f, 0.0185407f, -0.0144228f, 0.0195944f, 0.0105867f, 0.0108527f, 0.0318328f, -0.07468f, 0.0640258f, -0.0166149f, -0.0161666f, 0.0270572f, -0.00831346f, 0.0213354f, -0.0331297f, 0.0314013f, -0.0295451f, -0.0309544f, 0.00883464f, -0.000784053f, 0.00228157f, 0.030596f, -0.0169894f, -0.0723077f, 0.0142356f, -0.042197f, -0.0273198f, 0.0607149f, 0.0824823f, 0.0722077f, -0.0207748f, -0.0090944f, 0.0268541f, 0.0273479f, 0.00481306f, -0.00487477f, -0.0183224f, -0.0126787f, 0.0311318f, -0.0985153f, -0.0152497f, 0.00489618f, -0.0141078f, -0.0060658f, -0.000568589f, -0.032613f, 0.00976906f, -0.0462634f, -0.0259696f, -0.0786609f, -0.0153404f, 0.0249492f, 0.00292531f, -0.0255124f, 0.0202219f, 0.0304817f, -0.0177191f, -0.0135411f, -0.0064023f, 0.048916f, 0.0348483f, -0.00747575f, 0.0256531f, -0.0264167f, -0.027836f, 0.026632f, -0.0408624f, 0.0405082f, 0.0435032f, -0.0481381f, 0.0232822f, 0.0406269f, -0.104934f, 0.032984f, 0.00642478f, -0.0123055f, 0.0323379f, 0.0262914f, -0.00313157f, -0.0307961f, -0.059502f, 0.043095f, -0.0842975f, -0.0634201f, -0.0069968f, -0.0269704f, 0.0525556f, -0.0145985f, -0.026517f, 0.0287775f, -0.00225143f, 0.00998218f, -0.0208695f, 0.00038333f, -0.0179813f, 0.0299511f, -0.0270286f, -0.0215702f, 0.00986492f, -0.121571f, 0.0374826f, 0.0280122f, -0.0349332f, 0.00798409f, 0.00126605f, 0.0544963f, -0.00189064f, -0.0770879f, -0.00792704f, 0.0613617f, 0.0133352f, 0.0303873f, -0.000380032f, 0.0189077f, -0.0194632f, -0.00659714f, -0.0571043f, 0.041608f, -0.0141942f, 0.012823f, 0.00537086f, 0.000970999f, 0.0332154f, 0.0570762f, -0.0137126f, 0.0101087f, -0.00108052f, -0.0265809f, -0.0247709f, -0.00362676f, -0.0148946f, 0.013131f, -0.00308769f, -0.158096f, 0.00257066f, -0.0143705f, 0.0888035f, 0.00916709f, 0.00514034f, -0.0227268f, 0.134988f, -0.0492885f, 0.0022784f, -0.0144922f, 0.0256463f, 0.0246127f, -0.0242015f, -0.0270194f,
        0.0236487f, -0.00133765f, -0.023996f, 0.0121123f, 0.0473768f, -0.0229827f, 0.0620781f, 0.0348273f, 0.0118778f, -0.0358558f, -0.00418959f, 0.026328f, 0.00159447f, -0.0285201f, 0.0242085f, 0.024281f, -0.120022f, 0.00322402f, -0.00124464f, -0.00395719f, 0.00586048f, 0.0264264f, 0.0202582f, -0.0172882f, 0.0167585f, 0.00926656f, 0.00103096f, 0.00249462f, 0.00288184f, -0.00771514f, 0.0255329f, 0.0516628f, -0.0170072f, -0.00388561f, -0.00997277f, 0.0355019f, 0.000978238f, -0.144348f, -0.00646585f, -0.013882f, 0.033804f, -0.0377087f, 0.00771159f, -0.0061665f, 0.0237085f, -0.0122598f, 0.0771705f, -0.0542605f, -0.0292168f, -0.0110855f, 0.00780249f, -0.0262439f, -0.0170252f, 0.0232333f, 0.0221474f, -0.000682905f, 0.0456239f, 0.00516233f, -0.0356498f, 0.0433573f, -0.0725911f, 0.122393f, -0.000836771f, 0.0154195f, -0.00217232f, -0.0458872f, 0.0576701f, 0.0347757f, 0.00437707f, 0.0167836f, -0.024089f, 0.00395376f, 0.0226754f, -0.000325613f, -0.0119747f, 0.0166885f, 0.0133881f, -0.00825686f, -0.0115485f, -0.0256805f, -0.013069f, 0.029991f, -0.0104672f, 0.0468771f, 0.018202f, -0.0499781f, -0.0150365f, 0.0351706f, 0.000881884f, 0.0257364f, -0.00567146f, -0.0125245f, -0.00638529f, 0.00949407f, -0.00206895f, -0.00294736f, -0.00599403f, 0.0100478f, -0.0708312f, 0.0164853f, -0.00509979f, -0.0820398f, 0.00301894f, -0.011352f, -0.103304f, 0.0361376f, -0.00276168f, 0.0140668f, 0.0182486f, -0.0224722f, 0.00670642f, -0.00173934f, -0.0763404f, 0.00545386f, -0.0451032f, 0.258199f, -0.000526159f, -0.00244376f, -0.0070213f, 0.0136966f, 0.00651444f, 0.00336226f, 0.0129456f, -0.00535145f, -0.0337439f, -0.0488545f, 0.0363396f, -0.000131419f, -0.0442874f, -0.00468587f, -0.00406768f, -0.0170205f, -0.0192772f, -0.00277597f, 0.0212662f, 0.0767458f, -0.0198272f, 0.00671115f, 0.00387314f, -0.00222632f, 0.017668f, -0.0152864f, -0.00217823f, -0.0302261f, 0.0201784f, 0.00912841f, 0.0418803f, 0.00397826f, -0.0171634f, 0.0562426f, -0.00595202f, 0.0317872f, 0.00277863f, -0.0198806f, -0.0105047f, -0.0078311f, -0.00416702f, 0.0284072f, 0.00135271f, 0.00845078f, 0.0125683f, -0.00724979f, 0.0567957f, 0.0255109f, 0.002417f, 0.0114722f, -0.0229208f, 0.00542141f, 0.000680912f, -0.0124263f, -0.0973681f, 0.0429572f, -0.00896565f, 0.00102447f, 0.0209145f, 0.0365617f, 0.00698999f, 0.0611891f, -0.0021814f, -0.00791606f, 0.0636013f, -0.0503155f, 0.041678f, -0.00722059f, -0.00547887f, 0.00243705f, -0.0177814f, -0.12321f, 0.0569086f, -0.00487058f, 0.0123446f, 0.0015868f, -0.0272469f, 0.0180903f, 0.0104843f, 0.0105209f, 0.00808024f, -0.0662313f, -0.0499085f, -0.0297908f, 0.00678693f, 0.0158422f, -0.0149847f, -0.212685f, -0.029142f, -0.0216139f, 0.0197027f, -0.00509483f, 0.0406666f, -0.00101148f, 0.0137954f, 0.0292058f, 0.0261623f, 0.0879647f, -0.0120199f, 0.0276628f, -0.00208332f, 0.00630364f, -0.00283301f, 0.0313885f, 0.00132789f, 0.00430711f, 0.131565f, 0.00856252f, -0.0451589f, 0.0151607f, -0.00609563f, 0.104563f, 0.0503204f, -0.00188153f, -0.00152094f, 0.0331939f, -0.0268272f, -0.0720271f, 0.0120254f, 0.00428272f, -0.010781f, -0.0235618f, -0.0599427f, -0.0128298f, -0.039684f, 0.0124311f, -0.00907946f, -0.0219339f, -0.00574204f, 0.00290369f, -0.0397143f, -0.0306637f, 0.0046412f, -0.102802f, 0.02052f, 0.0177221f, -0.000307451f, -0.663219f, -0.00099111f, -0.00863413f, -0.0648291f, 0.141571f, -0.0264896f, -0.00967159f, -0.0105556f, 0.00667919f, 0.019933f, -0.0081883f, -0.0256497f, -0.0425081f, -0.00260382f, -0.00437219f, 0.0181059f, 0.0588014f, -0.0156841f, -0.0992774f, 0.0577409f, -0.0112435f, 0.0118955f, -0.01259f, -1.68039e-05f, -0.0231843f, -0.0715207f, 0.00562568f, 0.00659099f, -0.00432696f, 0.0402245f, -0.0132643f, 8.8306e-05f, 0.00698941f, -0.0695019f, -0.0112349f, 0.0696259f, -0.142201f, -0.0227633f, -0.019462f, -0.0518398f, -0.0213576f, 0.0148991f, 0.0344155f, -0.0131575f, -0.012708f, -0.00177817f, -0.00639755f, -0.000887201f, -0.0257106f, -0.0247181f, 0.00548285f, 0.0290425f, 0.122557f, -0.00347772f, 0.0268244f, -0.00612725f, -0.0196236f, -0.0472946f, 0.00890478f, 0.000844572f, 0.0154442f, 0.024701f, -0.0306896f, 0.0231992f, 0.0425512f, -0.0302086f, 0.0319046f, 0.0310391f, -0.00796268f, -0.0411025f, 0.00749199f, -0.0374908f, -0.0108962f, 0.0293042f, 0.00369268f, -0.0138972f, -0.00285899f, -0.0473339f, 0.00105261f, 0.0269907f, -0.0314717f, -0.0538936f, 0.0837861f, -0.0145771f, 0.0345362f, 0.222726f, -0.034146f, -0.0154113f, 0.0519213f, 0.0351403f, -0.0609869f, 0.0181544f, -0.0165051f, 0.00702428f, -0.0109979f, -0.00444243f, -0.018915f, -0.027162f, 0.00253407f, 0.0133815f, -0.000469394f, 0.109107f, 0.0153356f, 0.00683112f, 0.0128685f, 0.0282692f, -0.0384653f, 0.000389417f, 0.106818f, 0.0799349f, 0.0567321f, 0.0479257f, 0.00394279f, -0.00575818f, -0.575371f, -0.0118667f, 0.00356253f, -0.0399865f, -0.0217626f, -0.019511f, 0.0108772f, 0.0134627f, -0.000487889f, -0.00162015f, -0.0268957f, 0.0158162f, 0.0124589f, 0.0514896f, 0.0391116f, -0.02102f, 0.0289451f, -0.0162062f, 0.0295524f, 0.0240599f, 0.00653552f, -0.0296798f, -0.0614426f, 0.00678693f, -0.0126935f, -0.0259306f, -0.0270236f, -0.005202f, -0.027559f, -0.00571665f, 0.01303f, -0.0176816f, 0.00828625f, -0.0159388f, 0.016197f, -0.0685197f, 0.0359586f, -0.0149305f, -0.0100357f, -0.054005f, 0.0405895f, -0.0436483f, -0.0196033f, 0.0205626f, 0.0601753f, 0.00745636f, 0.00526461f, 0.00770411f, -0.00536197f, -0.0196271f, -0.00742883f, 0.0673765f, 0.0225239f, 0.0330661f, -0.0197954f, 0.0635232f, -0.00196483f, -0.0160432f, 0.0274051f, 0.0249642f, -0.0215083f, 0.00376016f, 0.0484418f, -0.0339058f, -0.00930553f, 0.000391001f, 0.0489547f, 0.00680175f, 0.0121302f, -0.0159317f, -0.00746274f, 0.00762586f, 0.0151285f, -0.00984925f, 0.00967698f, -0.063813f, -0.00191317f, -0.0225768f, -0.0460198f, 0.0129389f, 0.022693f, -0.0331679f, -0.0252172f, 0.0152612f, -0.0615063f, 0.00776267f, 0.0890267f, -0.0218608f, 0.0164835f, -0.048754f, 0.0158734f, 0.00247796f, -0.0340838f, 0.0199824f, 0.0422744f, 0.00495236f, 0.00733676f, -0.693422f, -0.057195f, -0.042145f, -0.0894016f, 0.00573138f, 0.00168211f, -0.00815092f, 0.1004f, -0.00830388f, 0.0212194f, 0.00796229f, 0.0182782f, -0.00677567f, -0.0025772f, -0.0141583f, -0.0503938f, 0.00933939f, -0.0440368f, -0.0650577f, -0.0133163f, -0.0150479f, -0.128004f, -0.025883f, -0.0142512f, 0.0267747f, 0.0603829f, 0.0616747f, 0.00518816f, 0.0353825f, -0.0136665f, -0.0116953f, -0.0117363f, -0.00988685f, 0.0161024f, -0.0164802f, 0.0120735f,
        0.0115264f, 0.00956785f, -0.0348965f, -0.0115787f, 0.0441999f, 0.0345045f, 0.0134386f, -0.0337335f, -0.00245127f, -0.0610053f, 0.0043896f, 0.0019506f, 0.013525f, -0.0545739f, 0.0306072f, 0.105704f, -0.0610636f, 0.0184838f, -0.0121108f, -0.00898275f, 0.0264786f, 0.0351719f, 0.00565877f, -0.00984551f, 0.0349376f, 0.0065558f, 0.000771663f, 0.000747164f, 0.00623147f, -0.0100182f, 0.0147877f, 0.027002f, -0.0082708f, -0.00312388f, -0.031057f, 0.0352335f, 0.0102762f, -0.136548f, -0.00137814f, -0.0245331f, 0.0302073f, -0.050357f, -0.0055813f, -0.0035066f, 0.0159663f, -0.00413293f, -0.0220518f, -0.0378098f, -0.000528503f, -0.00883574f, -0.0160642f, -0.0976056f, -0.00949359f, 0.0667935f, 0.0152671f, -0.00275173f, -0.00305567f, -0.00027522f, -0.0358676f, 0.0613587f, -0.0621408f, 0.0603126f, -0.00382261f, -0.0162797f, 0.0627967f, -0.0338104f, 0.019684f, 0.0723154f, 0.0405459f, 0.0150282f, 0.0116941f, 0.0159087f, 0.0423308f, 0.000188638f, -0.0151563f, 0.0213552f, 0.0260785f, -0.000634076f, -0.00666879f, -0.0143571f, -0.0154005f, 0.0452614f, -0.0241995f, 0.00760913f, 0.00565907f, -0.0146403f, -0.00882357f, 0.109466f, 0.000185842f, 0.0530813f, -0.0167083f, -0.0132453f, 0.00510363f, 0.000928611f, -0.0231941f, -0.00849421f, -0.0127253f, 0.0143131f, -0.104331f, 0.0150856f, -0.0115339f, -0.0400927f, -0.00650179f, 0.00782663f, -0.0161432f, 0.00612369f, -0.0368485f, 0.0320765f, -0.000285285f, -0.0252538f, 0.00567933f, -0.00326235f, -0.0118118f, -0.0067807f, -0.0626707f, 0.0314245f, -0.00367115f, 0.0034559f, 0.00094028f, 0.012767f, -0.0376215f, -0.0102952f, 0.0236869f, 0.00184345f, -0.0418395f, -0.0542331f, -0.00655869f, -0.00491183f, -0.0167015f, -0.0135059f, -0.0126727f, -0.0262544f, -0.0235505f, -0.00927455f, 0.044421f, 0.0340354f, 0.0544527f, 0.0133111f, 0.00308665f, 0.00078136f, -0.0023735f, -0.0141342f, 0.00124783f, -0.0175074f, 0.0506524f, 0.0344784f, 0.016513f, 0.00434411f, -0.0224391f, 0.0865785f, -0.00372209f, -0.0103298f, -0.00164323f, -0.0143697f, -0.0125625f, -0.00602005f, -0.00435671f, -0.0097799f, -0.00277924f, 0.0124438f, 0.00866435f, 0.00456806f, 0.032294f, 0.00501145f, 0.0381001f, 0.0142146f, -0.0373586f, -0.0278584f, -0.0268059f, -0.0109542f, 0.0129881f, -0.0289077f, -0.00849425f, 0.00391238f, 0.0105073f, 0.0449334f, 0.00855353f, 0.0402285f, -0.00646413f, -0.00671409f, 0.013527f, -0.0528845f, 0.0319318f, -0.0113917f, -0.0113392f, -0.000316065f, 0.0412851f, -0.0162739f, 0.0137208f, -0.0163712f, 0.0349673f, 0.00457418f, -0.0198638f, 0.0765183f, -0.001026f, 0.0113388f, 0.00846672f, 0.0122229f, -0.0401006f, -0.00219702f, 0.00703645f, 0.0321573f, 0.000362714f, -0.24312f, -0.014646f, -0.00614563f, 0.0187569f, -0.00394876f, 0.0243838f, -0.00188284f, 0.0050112f, 0.0221267f, -0.00302741f, 0.0435336f, -0.0226377f, 0.0262879f, 0.0155468f, 0.0279725f, -0.00188527f, -0.00564561f, -0.00020769f, 0.0150204f, 0.13116f, 0.021348f, 0.00731956f, -0.0343524f, 0.00212442f, 0.0352829f, 0.526485f, -0.00325235f, -0.00250349f, 0.0161844f, -0.0453576f, -0.0154224f, -0.0407768f, 0.0031079f, -0.00879997f, 0.00831367f, -0.0461003f, -0.0249753f, -0.0173187f, 0.0510597f, 0.0221946f, -0.0149577f, 0.000957178f, 0.0111411f, 0.00876051f, -0.0220329f, -0.0046637f, -0.020372f, 0.00369127f, 0.039286f, -0.00385722f, 0.0115072f, -0.00474474f, -0.0141273f, -0.19162f, -0.0187427f, -0.00145075f, -0.00458649f, -0.00136821f, 0.0037382f, 0.0102019f, -0.0101349f, -0.0303892f, -0.697959f, -0.00391341f, -0.00169856f, 0.0454146f, -0.0300301f, -0.0387779f, -0.0249505f, -0.0183996f, -0.00471838f, -0.00533851f, 0.000305908f, -0.00737827f, -0.0143906f, -0.0612462f, 0.0117793f, -0.0296389f, -0.0045701f, 0.0974987f, -0.0222056f, -0.00917552f, 0.00540695f, 0.376f, -0.0369584f, 0.0818413f, -0.0806179f, -0.0591828f, -0.0292424f, 0.0175326f, -0.0141385f, 0.01833f, 0.0209717f, -0.0198613f, -0.0303378f, -0.00184021f, -0.095508f, 0.00121903f, 0.00795399f, -0.0660669f, -0.000692821f, 0.00370955f, 0.140168f, -0.000690335f, 0.0085036f, -0.0224978f, 0.0989872f, -0.103726f, -0.00133824f, 0.00176511f, 0.0226218f, 0.00723803f, -0.0136401f, 0.0136266f, 0.00908615f, -0.0421018f, -0.0535609f, -0.0230947f, -0.0338358f, -0.00108633f, -0.0356084f, -0.109221f, -0.014515f, 0.0077523f, 0.0139792f, -0.0248496f, -0.023008f, -0.0472426f, 0.0865438f, 0.000595621f, -0.0451802f, -0.0395005f, 0.0493621f, -0.00124904f, 0.0988936f, 0.0572095f, -0.0729679f, -0.00415711f, 0.161504f, -0.00328739f, -0.0133308f, 0.00799106f, -0.0163052f, -0.0209516f, 0.00308542f, -0.0129289f, -0.0510538f, -0.0122714f, -0.0362058f, 0.0683402f, -0.0126313f, 0.0263825f, 0.0168551f, 0.00470125f, 0.0204198f, 0.0145374f, -0.021401f, 0.00460656f, 0.085484f, 0.0781075f, 0.0251125f, 0.00791536f, -0.0189591f, -0.0431845f, 0.051558f, 0.017842f, 0.36608f, -0.0343333f, -0.0303445f, -0.0115494f, 0.0530173f, 0.0165506f, -0.0235855f, -0.052452f, -0.00888096f, 0.0221193f, 0.0386185f, 0.0353902f, 0.0246971f, -0.0122489f, 0.0512722f, 0.00400143f, 0.0255521f, 0.00548785f, 0.00233302f, -0.0253462f, -0.0966852f, 0.00378993f, 0.00350757f, -0.0310213f, -0.0279353f, -0.00233223f, -0.0220107f, 0.00163079f, -0.00717164f, 0.00659987f, -0.00608499f, -0.02305f, 0.00402512f, -0.32546f, 0.0706807f, 0.0274278f, 0.0267394f, -0.00604822f, 0.0361692f, -0.0515999f, 0.0369351f, 0.0124044f, 0.0716815f, 0.0053833f, 0.00673388f, 0.0250085f, -0.000686182f, -0.00550432f, -0.00231397f, 0.00181825f, 0.022164f, 0.0330005f, -0.00140523f, 0.0463948f, -0.0278037f, -0.0318544f, 0.0275073f, 0.0620945f, -0.0128747f, 0.0329174f, 0.0206743f, -0.0352932f, -0.00835452f, 0.0248623f, 0.119621f, -0.0292978f, -0.0132096f, -0.0302576f, -0.0178306f, 0.0209123f, 0.0229405f, -0.0236861f, 0.00108116f, -0.0799521f, 0.00532662f, 0.0127616f, -0.00190055f, 0.00847102f, 0.00451121f, -0.0637118f, -0.0302129f, 0.0119081f, -0.117328f, -0.00946109f, 0.0605782f, -0.0390624f, 0.0192556f, -0.0170363f, 0.0300991f, 0.0444662f, 0.0422317f, 0.0170539f, 0.0504948f, 0.0270332f, 0.00916911f, 0.0242343f, 0.00898315f, -0.0158267f, -0.0475899f, 0.0175909f, -0.000817633f, -0.0176624f, 0.0975135f, -0.00854145f, 0.0155055f, 0.00762038f, 0.0229743f, -0.0525053f, -0.0149161f, -0.0367894f, -0.104801f, 0.013039f, -0.120883f, -0.0715135f, -0.0193206f, 0.0158965f, -0.0748989f, -0.120509f, -0.0506567f, 0.0147239f, 0.107749f, 0.0659703f, 0.0220761f, 0.0242295f, 0.0180054f, -0.0111281f, -0.0171504f, -0.014431f, 0.083154f, 0.0241038f, 0.0115941f,
        0.0112054f, -0.208447f, -0.0871743f, -0.0362684f, -0.0110118f, 0.068481f, 0.0322887f, -0.0375058f, -0.0130676f, -0.101841f, 0.0479009f, 0.0459907f, 0.00208143f, -0.0880017f, 0.0160549f, -0.0533964f, -0.0336657f, -0.000403741f, 0.0274574f, 0.00649047f, -0.0278283f, -0.0254132f, 0.0467184f, -0.0375531f, 0.127941f, 0.0291329f, 0.00155753f, 0.00199031f, 0.0183402f, 0.155697f, 0.0500429f, 0.00407514f, 0.0229933f, -0.00482785f, -0.0220735f, 0.0390895f, -0.0863406f, -0.132777f, 0.00204372f, -0.0069423f, 0.0260759f, -0.031759f, -0.00107891f, -0.0218382f, 0.00464639f, -0.00370248f, 0.00721869f, -0.0152541f, -0.00113688f, -0.00731756f, -0.0459436f, -0.0122795f, -0.0212339f, 0.072953f, 0.0268922f, -0.00254329f, -0.00535364f, 0.0200235f, -0.019393f, 0.00740422f, -0.0515143f, 0.0410708f, -0.00789718f, -0.0633389f, 0.0544137f, -0.0580859f, 0.0325159f, -0.015541f, 0.0178216f, 0.289658f, -0.0234133f, -0.0074536f, 0.0255261f, 0.00291012f, -0.0219596f, 0.0246941f, -0.00560577f, 0.00899517f, 0.00914874f, -0.0254892f, -0.0521876f, 0.0629406f, -0.00645591f, 0.111561f, 0.0122516f, -0.0106223f, -0.0132192f, -0.0819937f, 0.0132221f, -0.00695472f, -0.0207924f, -0.0723628f, 0.0495887f, -0.0359372f, -0.04756f, -0.0288064f, -0.08486f, 0.285901f, -0.0527237f, 0.0401743f, 0.00317573f, -0.00912604f, -0.00509804f, -0.019646f, -0.0133663f, 0.00250147f, 0.00489291f, 0.017901f, 0.117288f, -0.0253837f, 0.0201622f, -0.0127631f, -0.000326688f, -0.0153231f, -0.0756543f, 0.113002f, -0.0181392f, -0.00927301f, 0.0726324f, 0.00722584f, -0.0730271f, 0.0245927f, -0.102462f, 0.0356965f, -0.0606429f, -0.0444952f, -0.0166311f, 0.00795211f, -0.00189904f, -0.0158499f, -0.0204771f, -0.0472794f, -0.0079858f, -0.0501545f, 0.102751f, 0.0584957f, 0.0372233f, 0.00862791f, 0.00449617f, -0.0237138f, 0.00679621f, -0.0152089f, -0.00387291f, -0.126512f, -0.0284672f, -0.0684034f, 0.0303137f, -0.0162955f, -0.0581197f, -0.220276f, -0.00417518f, -0.0689113f, -0.017655f, -0.0224894f, 0.0357768f, 0.0133865f, 0.022937f, 0.0472434f, -0.00953042f, -0.0159915f, 0.00998823f, 0.00600883f, 0.0533401f, 0.194183f, 0.477756f, 0.0191196f, 0.0227464f, -0.00284643f, -0.13471f, 0.0769816f, 0.01241f, -0.0497929f, -0.0935632f, 0.0292851f, 0.0178327f, 0.104592f, -0.0467304f, -0.00100124f, -0.0401962f, -0.0224538f, -0.00678469f, -0.073481f, 0.227438f, -0.00830996f, 0.073789f, -0.0239749f, 0.154952f, -0.0544236f, 0.0156297f, 0.19281f, 0.0326588f, -0.00926173f, -0.0288493f, 0.0228173f, 0.0186095f, 0.0415022f, 0.0290895f, -0.00247426f, -0.0898812f, 0.0274265f, 0.0393059f, 0.0222607f, 0.019877f, -0.150684f, -0.262853f, -0.0894445f, -0.0205114f, -0.00142168f, 0.126473f, -3.85201e-05f, 0.0356633f, 0.0269576f, 0.0157574f, -0.0432543f, 0.0279592f, 0.024804f, -0.0267448f, 0.0191669f, -0.0040675f, 0.0139007f, 0.00963236f, -0.0110146f, 0.137714f, 0.0166686f, 0.0200946f, -0.0611695f, -0.0639973f, 0.0055134f, 0.042783f, 0.0271225f, -0.0468356f, 0.0247138f, 0.0103724f, 0.00932251f, -0.0140851f, 0.0358128f, -0.0059887f, 0.0386251f, -0.00545864f, 0.0596616f, -0.0379678f, 0.0116168f, -0.0113317f, -0.0299328f, 0.0217457f, 0.0063076f, -0.00526829f, -0.012835f, 0.0163333f, -0.0390477f, 0.0108823f, 0.127479f, -0.00949771f, 0.000669599f, -0.00832522f, -0.00771118f, -0.554012f, -0.259737f, 0.00827122f, -0.000538992f, 0.0152035f, 0.05717f, 0.00494831f, -0.0414577f, 0.0166355f, 0.0400496f, -0.0114314f, -0.0214246f, 0.00867137f, -0.0404191f, -0.0166356f, 0.0428265f, 0.0146152f, 0.00234592f, -0.0864799f, 0.0226774f, 0.00508847f, 0.0203778f, -0.0583453f, -0.00666855f, -0.127756f, -0.00862127f, 0.0452925f, -0.0831513f, -0.00326817f, 0.00995622f, 0.116901f, -0.0877858f, 0.112396f, -0.102312f, -0.105516f, -0.0259396f, 0.00757632f, 0.00122858f, 0.0103624f, 0.0457345f, -0.0242102f, -0.0583132f, -0.012498f, -0.313943f, 0.0069556f, -0.0319396f, 0.0172862f, -0.00853725f, 0.0116005f, 0.125311f, 0.00419865f, 0.0476964f, 0.00896339f, 0.00977134f, -0.0925261f, 0.0156905f, -0.018496f, 0.0196972f, 0.0157389f, -0.00196949f, 0.0145061f, -0.0606428f, -0.0694258f, 0.0709404f, 0.00871243f, 0.00455373f, -0.00558034f, -0.0824924f, 0.0011513f, 0.0384797f, 0.00638306f, 0.00363507f, -0.0606946f, -0.0774373f, -0.020545f, 0.0937525f, -0.00557294f, -0.0987101f, -0.0864387f, -0.0108511f, -0.0149365f, 0.0481765f, 0.036998f, -0.112909f, -0.00983293f, 0.135054f, 0.071086f, 0.019128f, 0.00687174f, -0.00651517f, -0.0349884f, -0.00583317f, -0.0110052f, 0.0398168f, -0.0141334f, -0.0344924f, -0.00134893f, -0.0270122f, -0.000114596f, 0.0220215f, -0.0321631f, 0.0329176f, 0.0261847f, 0.170964f, 0.0083325f, -0.0209986f, -0.00422142f, 0.00124639f, -0.000368193f, 0.00871341f, -0.0488562f, 0.0170233f, 0.0236273f, -0.0163899f, -0.120393f, -0.151225f, -0.00206636f, 0.105974f, -0.00312998f, 0.0290657f, -0.014112f, -0.0107348f, 0.032648f, 0.026346f, 0.0817057f, 0.0319139f, 0.00208954f, 0.0860523f, 0.0385837f, 0.115668f, 0.0399777f, 0.00339539f, -0.00545459f, 0.0598012f, 0.00298756f, 0.0148942f, -0.0227489f, -0.0139737f, -0.00473305f, -0.0326132f, -0.0229166f, 0.0207593f, 0.00277288f, -0.00719371f, -0.0202886f, -0.00475025f, 0.00617697f, 0.0162748f, 0.124789f, 0.0101917f, -0.0547861f, 0.0414249f, -0.0484098f, -0.0963767f, 0.0484124f, -0.00538394f, 0.00789277f, -0.0102249f, 0.104348f, 0.00192805f, 0.00647828f, -0.0461272f, -0.00422982f, 0.0325315f, 0.0211869f, 0.0120108f, 0.0362735f, -0.100353f, -0.106846f, 0.0453949f, 0.108593f, -0.00433587f, 0.0477131f, -0.011734f, -0.00221842f, -0.0186096f, 0.0176472f, 0.0535756f, -0.00753715f, -0.00288954f, 0.0351324f, -0.000401414f, 0.0260439f, 0.0353749f, -0.0214858f, 0.000924544f, 0.000524206f, 0.0411247f, -0.0106592f, 0.0429043f, 0.0430829f, 0.029413f, -0.00455873f, -0.0157798f, 0.0105745f, -0.10904f, -0.0209529f, -0.0605732f, 0.0151213f, 0.0293344f, -0.030329f, 0.0388919f, 0.0830521f, -0.0636824f, 0.0834155f, -0.0213396f, -0.0029129f, 0.0130443f, 0.0276463f, 0.0168069f, 0.0131674f, 0.00569299f, 0.0319241f, -0.215148f, -0.080939f, 0.033579f, -0.0317194f, 0.0329596f, -0.000564258f, 0.0486169f, -0.0763688f, -0.0114993f, -0.0945774f, -0.0518434f, 0.0208386f, -0.059539f, -0.109562f, 0.0544319f, -0.0264226f, -0.090276f, -0.0850728f, -0.0434345f, -0.00627017f, 0.0531473f, 0.214103f, -0.0206121f, 0.0996398f, 0.155764f, -0.00199676f, -0.0115997f, -0.0355156f, 0.113538f, 0.0365645f, 0.0733962f,
        -0.00286558f, -0.000197294f, -0.00342685f, 0.0153883f, -0.0289065f, -0.00108885f, 0.0387835f, 0.00578341f, -0.0262422f, 0.0582573f, -0.0156079f, 0.00150583f, -0.0362458f, 0.0104373f, 0.0113533f, -0.0386981f, -0.00206408f, 0.00844815f, -0.053396f, -0.00516133f, -0.0270117f, -0.27301f, 0.0846544f, -0.00616813f, 0.0871727f, 0.00543487f, 0.00184716f, 0.000983837f, 0.0179262f, 0.0818094f, 0.00281717f, 0.000740774f, -0.19657f, -0.00320105f, -0.00607788f, 0.0319331f, -0.0235907f, -0.0270345f, -0.00849474f, -0.0110374f, -0.0746362f, -0.00860617f, -0.0237307f, -0.00439848f, 0.0264778f, -0.00958103f, 0.019552f, -0.00263648f, 0.0296515f, -0.00113696f, -0.124069f, -0.0114847f, 0.00308178f, 0.0198766f, 0.0900992f, 0.00727182f, -0.0940811f, 0.0216224f, -0.0459814f, 0.017883f, -0.0572063f, 0.0627439f, -0.00563362f, -0.0648773f, -0.00153448f, -0.0913932f, 0.0230685f, 0.00374598f, 0.034431f, 0.112274f, -0.326763f, 0.0153011f, 0.00631464f, 0.00570424f, -0.0595568f, -0.0114755f, -0.0054627f, -0.000223818f, -0.00287856f, -0.0252934f, -0.0108832f, 0.0186247f, -0.0140906f, -0.0239924f, -0.0332168f, -0.00818134f, -0.0261136f, 0.0112761f, -0.0570478f, -0.0484226f, -0.00280576f, -0.140804f, -0.0192205f, 0.0193394f, 0.0043392f, -0.0096851f, -0.0238295f, 0.0496011f, -0.00870349f, 0.0271804f, 0.00735628f, 0.00931979f, -0.000362012f, -0.00038859f, 0.098518f, -0.0510564f, -0.00233872f, 0.00517725f, 0.0101231f, -0.0331861f, 0.0441328f, -0.00924161f, -0.0059294f, -0.0159056f, -0.0810096f, 0.203707f, 0.00935022f, 0.00920423f, 0.0427866f, 0.0270535f, -0.0613705f, 0.0281747f, -0.0292151f, -0.011845f, -0.560809f, -0.0430764f, 0.0249193f, 0.001065f, 0.0495798f, -0.00604974f, -0.0115863f, -0.0841969f, -0.0400231f, -0.0234006f, -0.099013f, 0.0434646f, 0.000907694f, 0.0125445f, 0.0118042f, -0.00467421f, 0.00886041f, 0.0296945f, 0.0324396f, -0.0114072f, 0.0988003f, 0.00847453f, 0.0464346f, -0.00464305f, -0.0289332f, 0.0590643f, -0.0350208f, -0.0899201f, -0.0159029f, -0.0648027f, 0.00696909f, -0.00101221f, 0.0140877f, -0.0855302f, -0.000846032f, -0.0256277f, 0.00854884f, -0.00292961f, 0.209544f, 0.0872828f, 0.0246488f, 0.0291603f, -0.0784974f, 0.00920441f, -0.011242f, -0.0297102f, 0.0152799f, -0.0428288f, -0.0651387f, 0.0138869f, 0.0139815f, 0.0836656f, 0.0361113f, -0.0635471f, -0.0160178f, -0.0220017f, 0.234027f, -0.0400384f, 0.186927f, -0.0295061f, 0.00130944f, -0.0287178f, -0.0214042f, -0.0285818f, 0.0222618f, -0.00368823f, 0.0601194f, -0.0188088f, -0.0146725f, 0.0157483f, 0.21603f, 0.056817f, -0.20685f, -0.0254415f, 0.00525571f, 0.00219887f, 0.0530388f, 0.0607272f, 0.0061378f, -0.113869f, -0.16334f, -0.0464161f, -0.00694523f, -0.00488537f, 0.0286918f, 0.00290496f, 0.178755f, 0.0109929f, 0.110835f, -0.0642967f, -0.0333608f, 0.00169389f, -0.00546941f, 0.00973807f, -0.00576067f, -0.0205257f, 0.0511577f, -0.0266243f, 0.109812f, 0.0471989f, 0.0996845f, 0.0135877f, -0.0794984f, 0.0649346f, 0.0303168f, -0.0011697f, 0.00521801f, 0.0626395f, -0.00297682f, 0.0266726f, -0.000223535f, 0.0116355f, -0.0108245f, 0.000611158f, 0.00728507f, 0.0239288f, -0.00188282f, 0.0150957f, -0.040548f, -0.0589448f, 0.0328252f, -0.0915972f, 0.0805046f, -0.00811939f, 0.0772469f, -0.0716012f, 0.000604462f, 0.047583f, 0.0334997f, -0.000381467f, -0.00726828f, 0.00027943f, -0.0427843f, -0.0568598f, 0.0147649f, -0.00348073f, 0.00288838f, 0.00979242f, -0.00538436f, -0.024106f, 0.00541673f, 0.00529046f, -0.00278852f, -0.0222607f, -0.00626747f, 0.0973789f, -0.0795939f, 0.105127f, -0.337742f, 0.0172115f, 0.00255328f, -0.0330435f, 0.0063678f, 0.0471297f, -0.050865f, -0.00217128f, 0.0139913f, -0.00278459f, 0.0452206f, -0.0122722f, 0.00537665f, 0.0068003f, -0.0241691f, -0.00537261f, 0.00198657f, 0.0288662f, -0.0673232f, -0.00391073f, 0.0160158f, -0.0148616f, 0.00889894f, 0.0278599f, -0.0259723f, -0.0464762f, -0.0699778f, 0.0855682f, -0.00447207f, -0.105144f, -0.000995281f, -0.0146742f, -0.49647f, 0.0685417f, -0.000740646f, 0.0278313f, -0.00761982f, 0.0475931f, -0.0645097f, 0.119236f, -0.0570179f, 0.00915969f, 0.0156965f, 0.101129f, -0.0274397f, 0.0317f, 0.435965f, 0.0895423f, 0.0228896f, 0.0537683f, -0.0312062f, -0.0316729f, 0.00405423f, -0.00417011f, 0.053186f, 0.0124111f, -0.0636419f, -0.059223f, 0.00212677f, -0.00180764f, -0.0184438f, -0.00539991f, -0.0216965f, -0.0297828f, -0.00665945f, 0.0659594f, 0.109878f, -0.0859683f, -0.0195527f, 0.0856906f, 0.113261f, 0.0901811f, 0.00573377f, 0.0357797f, -0.0261576f, 0.0127095f, 0.00452054f, 0.0160191f, 0.0674667f, -0.0187489f, 0.00896214f, -0.00895184f, 0.388793f, 0.0155203f, -0.206128f, -0.0134212f, 0.0159576f, 0.240592f, -0.0244503f, 0.0595618f, 0.0056212f, -0.0505254f, 0.160077f, 0.0021605f, 0.111341f, -0.664956f, 0.031356f, -0.00658282f, -0.431486f, -0.0241319f, -0.437714f, 0.0186697f, 0.0143805f, -0.0139802f, -0.00777148f, 0.0223012f, -0.0458929f, 0.0103136f, 0.0203269f, -0.0121667f, -0.00358236f, -0.0347832f, 0.0310102f, 0.0940264f, 0.0402878f, 0.0779475f, 0.085935f, 0.0506573f, 0.0125433f, 0.00945608f, 0.00711064f, -0.0157027f, -0.00267093f, -0.0460969f, 0.00133153f, 0.0510218f, 0.0568231f, 0.00654478f, -0.0148599f, -0.00556127f, 0.0984337f, 0.0012008f, 0.0401073f, -0.00218267f, -0.0913605f, 0.0250143f, 0.0269926f, -0.00189873f, 0.145338f, -0.0106285f, 0.128684f, 0.0182833f, -0.0104387f, 0.058272f, 0.054818f, -0.0204594f, 0.0514151f, -0.0114196f, 0.0121938f, -0.0135972f, 0.00423344f, 0.0268584f, -0.0233103f, 0.0149913f, 0.00556167f, 0.175006f, 0.0460865f, -0.0531133f, -0.00530817f, 0.00775018f, -0.00568381f, 0.00309299f, 0.00404426f, 0.0611169f, 0.04162f, 0.0620172f, 0.0113454f, 0.0556293f, -0.000326539f, -0.0136839f, -0.00373327f, 0.0962103f, -0.0169842f, 0.0247842f, 0.0442757f, 0.0244144f, -0.0176649f, -0.00554654f, -0.0050203f, -0.0177601f, -0.02368f, 0.0243078f, -0.0571087f, 0.0184628f, -0.0841841f, 0.0331607f, 0.0279732f, -0.0822138f, 0.0293232f, -0.0722001f, 0.0163439f, 0.0191851f, 0.414194f, 0.456304f, 0.097353f, 0.033467f, -0.010367f, -0.00362604f, -0.00940526f, 0.0541993f, -0.0126803f, -0.0284043f, -0.126488f, 0.0276941f, -0.0072592f, -0.0112239f, 0.200614f, -0.0674165f, 0.0152713f, -0.0543701f, -0.0742834f, -0.0453187f, -0.0254072f, -0.0692672f, 0.0332971f, -0.0228297f, -0.000965714f, 0.0732683f, 0.0640799f, 0.00158938f, 0.047803f, -0.00266977f, -0.0100275f, -0.00643167f, -0.0383495f, -0.00409583f, 0.0385844f, 0.0659188f,
        0.0063133f, -0.00408226f, 0.121465f, 0.0301708f, -0.0181853f, 0.0601681f, 0.00325393f, 0.10642f, -0.0275263f, -0.0194839f, -0.0252979f, 0.0217105f, 0.0386137f, 0.0112424f, 0.0430641f, 0.0730034f, 0.0354242f, 0.013652f, -0.0293887f, 0.142649f, -0.0690173f, -0.0961422f, 0.0442838f, 0.0452969f, 0.118274f, 0.0323701f, 0.0187156f, 0.5255f, 0.0118736f, 0.225357f, -0.0130602f, -0.0104742f, -0.07411f, -0.114514f, -0.0436895f, 0.00986579f, -0.0838205f, -0.101698f, -0.00483559f, -0.00391671f, -0.0699783f, -0.0195803f, 0.0459022f, -0.0091508f, 0.0073998f, -0.0577818f, 0.0674949f, 0.0137614f, 0.0715333f, 0.00271481f, -0.00891188f, -0.0212177f, 0.0437716f, 0.0257086f, 0.0345469f, -0.180349f, -0.0603965f, -0.147289f, -0.00330522f, 0.0067096f, -0.0179399f, 0.0182082f, -0.0270762f, 0.0402878f, -0.0166916f, -0.0948335f, 0.029574f, 0.0969981f, 0.0529901f, 0.00293059f, -0.154666f, 0.0407095f, 0.0316545f, -0.0062415f, -0.0351574f, -0.0147547f, -0.0135113f, 0.00357694f, 0.0517612f, -0.101499f, -0.00291564f, -0.0056001f, -0.00857672f, -0.0101505f, -0.0323477f, -0.0263152f, -0.0116552f, 0.0247082f, 0.0227123f, -0.10951f, -0.0328793f, 0.411161f, -0.0130315f, -0.0227835f, 0.0106074f, -0.00307627f, 0.00495261f, 0.0545998f, 0.000595861f, -0.0242671f, 0.0299187f, 0.00166324f, -0.00666328f, -0.0078437f, 0.0280452f, -0.16448f, -0.0143541f, 0.026909f, -0.193269f, -0.0355148f, 0.0118665f, -0.0365043f, -0.00810059f, -0.0352678f, -0.0630561f, 0.0280126f, 0.30164f, 0.0875995f, 0.0694396f, 0.0103573f, -0.0283321f, -0.621525f, -0.0445668f, -0.0148087f, -0.313831f, -0.00408616f, 0.0349075f, 0.0231337f, 0.142115f, 0.00382164f, 0.0393434f, -0.108881f, -0.0101964f, -0.0303501f, -0.106503f, 0.0308691f, -0.0197364f, 0.0091609f, 0.00739707f, -0.021932f, 0.00100097f, 0.00910001f, -0.0272304f, 0.0244325f, -0.0534487f, -0.0124806f, 0.102616f, -0.0300018f, -0.0371498f, -0.0484335f, -0.0434477f, -0.0806446f, -0.0323094f, 0.0210301f, 0.016248f, 0.0884761f, 0.0521384f, -0.306267f, -0.0181587f, 0.0638134f, 0.00266205f, 0.0659853f, 0.0215718f, 0.030898f, -0.010891f, 0.0265176f, -0.0440084f, 0.0334551f, -0.0404191f, -0.05042f, 0.0401076f, 0.00569889f, 0.0642698f, 0.0118167f, -0.152626f, -0.0383063f, -0.241934f, -0.14967f, 0.000835922f, -0.0176463f, 0.00669299f, -0.100216f, 0.0636827f, -0.0246564f, 0.0233452f, 0.00916313f, -0.0360494f, -0.0143271f, 0.00748104f, 0.00808922f, 0.120031f, -0.0139543f, -0.0895863f, -0.0414794f, 0.143243f, -0.0137803f, 0.0207675f, -0.0347851f, 0.0721874f, -0.0414808f, -0.116213f, 0.00107106f, 0.0103554f, -0.13586f, -0.290486f, 0.00166402f, -0.015201f, -0.00145561f, -0.0154914f, 0.00163743f, 0.0822632f, 0.08017f, 0.0710966f, -0.013158f, -0.0632138f, -0.0111834f, -0.0178201f, 0.0112061f, -0.00430423f, -0.0674515f, 0.214633f, -0.00585192f, -0.0351569f, 0.375032f, 0.0448701f, 0.0256456f, 0.0743934f, 0.0211866f, -0.00896532f, -0.0415844f, 0.0122347f, 0.0118991f, -0.0877453f, 0.0304085f, -0.00665392f, -0.00567859f, -0.00832385f, 0.00138205f, 0.0402719f, -0.00329125f, -0.0122391f, 0.0130672f, -0.0699987f, -0.0336706f, 0.0130345f, -0.256598f, -0.00998923f, -0.0732391f, 0.16722f, -0.0470782f, 0.016357f, 0.0118742f, -0.0706653f, 0.00409f, -0.0124226f, 0.000505835f, -0.0507414f, 0.00258108f, 0.0198879f, 0.000320695f, 0.0112645f, 0.00723067f, -0.0107117f, -0.00964231f, 0.014985f, -0.000720747f, -0.00563631f, -0.128197f, -0.00191921f, 0.100766f, -0.0177464f, 0.0910596f, 0.132686f, 0.0851709f, 0.0140803f, -0.0459295f, 0.00891749f, 0.0917738f, -0.0520881f, -0.00429575f, -0.0104893f, -0.0285219f, 0.0370703f, -0.0241567f, 0.0214466f, 0.0260263f, 0.112436f, -0.0221967f, 0.003362f, 0.00552892f, -0.0382231f, 0.00763609f, 0.0270099f, -0.028698f, -0.00121651f, 0.000527033f, -0.0406943f, -0.0840261f, -0.00983556f, -0.0288269f, 0.00269151f, -0.136611f, 0.0220631f, -0.00476321f, 0.0281217f, 0.0243983f, -0.00436437f, 0.00491977f, 0.0540143f, 0.0410553f, -0.00945594f, -0.0711867f, -0.011407f, -0.0290617f, 0.0077444f, -0.0194761f, -0.0353022f, 0.0242323f, 0.121606f, 0.136937f, 0.117977f, 0.0648052f, 0.000369128f, -0.0286182f, -0.000851573f, -0.0675435f, 0.0374786f, 0.0108061f, -0.00134871f, -0.0419874f, 0.0271549f, -0.21822f, 0.268321f, -0.00535237f, 0.011111f, -0.0614932f, 0.0500974f, 0.0900748f, 0.0334851f, -0.101783f, -0.00498551f, -0.0075128f, 0.00031712f, 0.0485839f, 0.000919265f, 0.0326066f, -0.023036f, 0.0096988f, 0.0178391f, 0.0861196f, 0.0466213f, -0.0299909f, -0.0991148f, -0.0230341f, 0.334094f, -0.0382573f, 0.0395579f, -0.00590484f, 0.0206429f, 0.246985f, -0.0283786f, 0.0598143f, -0.0353774f, 0.091151f, 0.0944889f, 0.00249664f, 0.202462f, -0.00569812f, 0.00865333f, -0.00812537f, -0.188173f, -0.0627191f, -0.28001f, 0.00917071f, 0.0506412f, 0.0010405f, 0.0678395f, 0.16542f, -0.00219039f, 0.0110519f, -0.00379539f, 0.00535911f, -0.00791708f, -0.000717427f, -0.0325235f, 0.0842137f, -0.020968f, 0.192455f, 0.0856024f, 0.132173f, -0.00232728f, 0.0647325f, 0.104932f, -0.0235684f, 0.00335134f, 0.00515333f, 0.192284f, 0.0592319f, 0.143246f, -0.00214825f, -0.168829f, -0.0149753f, 0.00881463f, 0.00489184f, 0.0030815f, -0.0645487f, -0.236596f, 0.0211161f, 0.428909f, -0.0184283f, 0.150971f, -0.00403509f, 0.0892136f, 0.0527521f, -0.00892411f, 0.257531f, 0.0159127f, -0.0153799f, 0.0299046f, 0.00748111f, 0.02268f, -0.0283898f, -0.0224564f, -0.00329609f, -0.0642335f, 0.0385503f, 0.00387719f, -0.0795388f, 0.0385978f, 0.0338672f, -0.00181007f, 0.500546f, 0.0174027f, -0.00941603f, 0.00119533f, 0.161396f, 0.0277067f, -0.0113644f, 0.00243689f, 0.0240222f, 0.00074696f, -0.00329644f, 0.00571551f, 0.353842f, -0.0345694f, 0.0954816f, 0.022245f, 0.0639779f, -0.0209006f, -0.0100804f, -0.0223871f, 0.00248849f, -0.0231191f, -0.105286f, -0.0150994f, 0.00230265f, -0.0295301f, 0.0119341f, 0.00911531f, 0.0540066f, 0.0076047f, -0.0945892f, 0.0196067f, -0.0357786f, 0.0719775f, -0.0972845f, 0.142406f, -0.18177f, 0.00491428f, 0.000342362f, -0.0186926f, 0.0489506f, -0.0333847f, -0.017827f, -0.00585373f, 0.0250148f, -0.0496847f, 0.00595432f, 0.180951f, -0.0459607f, -0.0360709f, -0.168328f, -0.0724864f, -0.161582f, 0.0156965f, -0.0463856f, 0.00603378f, -0.0396591f, 0.100121f, 0.00849666f, 0.0438226f, 0.0247446f, 0.0309354f, -0.0876779f, -0.0223912f, 0.0149475f, -0.0619022f, -0.0198987f, 0.0258675f, 0.0760512f,
        0.0237833f, 0.00298876f, 0.0487694f, 0.00950606f, -0.074622f, 0.0192038f, -0.0202395f, 0.105125f, -0.0154085f, 0.0355691f, 0.00281225f, 0.00531638f, 0.0101454f, 0.0510713f, 0.0313131f, -3.24692e-05f, 0.0563302f, -0.00384794f, -0.0967057f, -0.00911184f, -0.034748f, -0.00885298f, -0.00145702f, 0.00841001f, -0.00386897f, 0.00954715f, 0.0060942f, -0.00779779f, 0.0341911f, 0.0373562f, 0.000677265f, -0.0620633f, 0.00208294f, -0.0215586f, -0.085074f, 0.0143441f, -0.0186877f, 0.00127867f, -0.01249f, -0.00504883f, -0.00104019f, 0.0121985f, 0.000512828f, -0.00772995f, 0.00468516f, -0.0139477f, -0.0211804f, 0.210879f, 0.00785329f, -0.000516933f, -0.00212956f, -0.0162727f, 0.00414868f, 0.0109553f, 0.000250999f, -0.00637749f, -0.00108913f, -0.00648906f, -0.0123977f, 0.0104616f, 0.0241319f, 0.0770632f, 0.00195405f, -0.00752428f, -0.0405081f, -0.0883033f, 0.0394711f, 0.0062544f, 0.0315002f, -0.0138193f, -0.0353362f, 0.00803457f, 0.0055575f, -0.00122304f, -0.00591179f, -0.000313378f, -0.00928775f, 0.00167335f, 0.00110711f, 0.0102733f, -0.0102128f, -0.0332447f, -0.0050578f, -0.0365285f, 0.00129188f, -0.00545454f, -0.0488076f, -0.0522689f, -0.0028496f, 0.0269232f, -0.00264586f, 0.00549725f, 0.0937312f, -0.0097157f, 0.000703438f, -0.0316939f, 0.00265145f, 0.00747435f, 0.00703635f, -0.0498706f, 0.0260258f, 0.00486406f, 0.00831138f, 0.00331964f, -0.0116462f, -0.000328743f, -0.0193854f, 0.012874f, -0.0140591f, 0.00294906f, 0.167637f, -0.00563081f, 0.00047881f, -0.0132155f, -0.088562f, -0.00763682f, 0.00861545f, 0.0484862f, 0.118604f, 0.00888342f, -0.0480975f, -0.0108402f, -0.00768345f, -0.214419f, -0.045855f, 0.000607434f, 0.00143275f, 0.000233664f, 0.00111974f, 0.0283561f, -0.0137152f, 0.035663f, -0.0231469f, 0.0205628f, 0.0685008f, 0.0106492f, 0.00590557f, -0.00685771f, 0.00424108f, 0.000113577f, 0.00595773f, 0.00665598f, 0.000441705f, -0.00402036f, -0.0262544f, 0.00611645f, 0.0116063f, -0.00424871f, 0.0342696f, 0.0381022f, -0.0588067f, -9.04306e-05f, 0.013434f, 0.0049054f, 0.0123942f, -0.000403249f, 0.0504587f, -0.00181204f, 0.00841684f, 0.0187689f, 0.0174106f, 0.00611652f, 0.00976013f, 0.000955711f, 0.00209072f, -0.0257193f, -0.0127599f, 0.00699173f, -0.0153516f, -0.00193625f, 0.0528177f, 0.0170662f, 0.0746572f, 0.00809554f, -0.027025f, -0.0257472f, -0.00256271f, -0.0890082f, -0.00221022f, -0.00891542f, -0.00903598f, -0.0144857f, 0.0554675f, -0.00986486f, 0.00189685f, 5.93501e-05f, 0.00462237f, 0.00532594f, 0.00433364f, -0.003124f, 0.04f, -0.000328486f, -0.0648411f, -0.00377033f, 0.139774f, 0.00230164f, 0.0115385f, 0.0125043f, 0.148022f, -0.0284796f, -0.00155402f, -0.00387695f, 0.00829478f, -0.0471497f, -0.0015643f, -0.00582674f, -0.00431319f, 0.000878919f, 0.00687072f, -0.00301133f, 0.00398096f, -0.00563914f, -0.0026393f, -0.00377055f, -0.0609272f, -0.118688f, 0.00517703f, 0.0836725f, -0.012182f, -0.0512972f, 0.0119928f, 0.0247734f, -0.0427426f, 0.0341825f, 0.0698612f, 0.00279914f, -0.00847926f, -0.0226391f, 0.020679f, -0.00144619f, -0.0104832f, 0.0195441f, 0.000150691f, 0.0815801f, -0.00616593f, 0.00379428f, -0.00447982f, 0.00261409f, 0.0600844f, -0.0213836f, -0.00804557f, 0.00325642f, 0.00854879f, -0.0814344f, -0.027769f, -0.00191851f, 0.00536533f, -0.0164033f, -0.00257131f, -0.00205376f, -0.0200541f, -0.0128954f, -0.00532982f, 0.0022407f, -0.00130887f, 0.00425618f, -0.00845818f, -0.00126148f, -0.0107566f, 0.00104842f, -0.00435674f, 0.00433842f, -0.0109865f, 0.000301519f, 0.00589863f, -0.00851759f, -0.00137109f, -0.0256632f, 0.0120122f, -0.00451766f, -0.0132172f, 0.0204377f, 0.00862719f, -0.00529603f, 0.0007616f, -0.00779072f, 0.000307369f, 0.0161384f, 0.0140168f, -0.00223271f, -0.0234216f, 0.00152691f, 0.00407567f, -0.00575267f, -0.0169706f, 0.00373715f, -0.0130443f, 0.0149063f, -0.00592504f, -0.00101738f, -0.00432452f, 0.00608682f, -0.00623923f, -0.0048846f, 0.00141049f, -0.00787022f, -0.00325903f, -0.00925192f, 4.10188e-05f, -0.00650579f, -0.00344007f, -0.00507379f, -0.010943f, 0.0033921f, 0.0262149f, -0.0109309f, -0.00218072f, 0.00487267f, -0.00424018f, 0.0190863f, -0.0205672f, -0.00521787f, -0.749656f, 0.0045255f, -0.0111087f, -0.00594957f, -0.00784532f, -0.00218566f, -0.00261733f, 0.00115839f, 0.00810127f, -0.00685174f, -0.000515265f, 0.00996413f, 0.00908507f, -0.010911f, 0.0199673f, 0.00424915f, -0.0168506f, -0.0127626f, -0.0068238f, 0.0141051f, -0.0106615f, 0.00332799f, 0.00636155f, -0.0260333f, 0.00595097f, 0.0191085f, -0.0049198f, 0.00793315f, -0.00309666f, 0.0137166f, -0.00473366f, 0.0127659f, 0.000838826f, 0.0352708f, -0.00566433f, 0.00439918f, 0.00403144f, -0.0103773f, 0.000578005f, -0.00181792f, -0.0300049f, -0.00661571f, 0.0085107f, 0.00894339f, 0.00861617f, 0.00351911f, 0.016009f, -0.00165849f, 0.00140448f, 0.00854556f, -0.000467159f, 0.00526625f, 0.0113457f, -0.000892589f, -0.00943319f, 0.016298f, 0.0129145f, 0.00977724f, -0.00864554f, -0.0149309f, 0.0109739f, 0.00925517f, 0.00301191f, -0.00253138f, -0.0198261f, 0.00383641f, 0.00511284f, -0.0561408f, -0.0281949f, -0.00444545f, -0.00338158f, -0.00161292f, -0.00978353f, 0.00446439f, 0.000485823f, 0.000591379f, 0.00729576f, -0.024535f, 0.00937071f, 0.00193014f, 0.00812366f, -0.015649f, -0.00101637f, 0.0112705f, 0.00182169f, -0.00906464f, 0.0080621f, -0.0130414f, -0.000293886f, -0.00548405f, -0.00557287f, -0.00444211f, 0.000131822f, -0.0116247f, 0.00918694f, 0.00706824f, -0.00459982f, -0.00134241f, 0.00769962f, -0.000905408f, -0.00643464f, 0.00195699f, 0.0103661f, 0.0117231f, 0.00141366f, 0.013737f, -0.00475491f, -0.00389627f, -0.008428f, -0.00336822f, -0.0123985f, -0.00384732f, -0.00772105f, -0.00399041f, 0.00441658f, -0.0179348f, 0.00088589f, 0.00130237f, -0.00910743f, -0.000932973f, -0.000705488f, -0.00845157f, -0.00409019f, -0.00198943f, -0.00037801f, -0.0110968f, -0.00639611f, 0.00967489f, -0.00286205f, -0.00142743f, 0.00952024f, 0.0067011f, -0.00771389f, 0.000101275f, 0.00173372f, 0.000959312f, 0.00841471f, 0.00336334f, 0.00371336f, 0.00482025f, -0.00711383f, 0.00583148f, 0.0108545f, -0.000470039f, -0.0110626f, 0.00324574f, 0.025979f, 0.0153801f, -0.00239289f, -0.0364105f, -0.0252222f, 0.00766028f, -0.000371992f, -0.00263989f, 0.0215774f, 0.0230998f, -0.00223724f, -0.000281751f, -0.00482297f, -0.0175295f, -0.00712851f, 0.0106509f, 0.00430235f, 0.00410187f, 0.00823292f, 0.00280169f, 8.28998e-05f, -0.00169138f, -0.00976853f, -0.00530213f, -0.00814388f, 0.0013187f, 0.00816157f, 0.00138731f, -2.68979e-05f, -0.0103893f, -0.0500543f, 0.000847671f, 0.00327953f, 0.00418289f, 0.0180997f, -0.00027566f, -0.00544788f, -0.0076323f, -0.00551657f, -0.00599236f, -0.0127374f, -0.0174632f,
        -0.000449777f, -0.000137405f, -0.0762075f, 0.000949166f, 0.0346124f, -0.0111424f, 0.0108357f, 0.0121679f, 0.0242749f, 0.052692f, -0.0017713f, 0.0053728f, 0.0128862f, -0.0162366f, 0.0125041f, -0.00602398f, 0.0107778f, -0.00323086f, -0.00914208f, -0.013884f, 0.00755173f, -0.0175622f, 0.00473339f, -0.015003f, -0.0238219f, 0.004502f, 0.00187154f, 0.0041163f, -9.36184e-05f, 0.00873372f, 0.0121869f, -0.020973f, -0.006006f, -0.0038208f, 0.00210471f, 0.00255549f, -0.0251856f, -0.0626372f, -0.0059258f, -0.0058662f, -0.0946306f, 0.00197436f, 0.00105865f, -0.0033595f, 0.0158977f, -0.0036025f, -0.00568902f, -0.0202577f, -0.000251319f, -0.0117895f, -0.0144239f, -0.0144024f, -0.0150431f, -0.0354826f, -0.0135123f, -0.000422157f, 0.0286438f, -0.000884989f, -0.00675718f, 0.013241f, -0.0118388f, 0.0321394f, -0.000803071f, 0.11408f, -0.00806301f, -0.00831608f, 0.0165189f, 0.016094f, -0.000449332f, -0.00695901f, 0.0437514f, -0.00172117f, 0.00180391f, -0.000859933f, -0.0144826f, 0.0262613f, -0.00194352f, -1.98829e-05f, -0.00902827f, -0.00400867f, -0.00600827f, 0.0120846f, -0.0162493f, 0.0418596f, 0.00131911f, -0.00631566f, 0.00270484f, -0.0950513f, 0.00726431f, -0.0169798f, -0.000554365f, -0.00256903f, -0.00885843f, 0.0104025f, 0.00590779f, -0.00175832f, 0.0168603f, 0.00964353f, -0.0180614f, 0.0213157f, 0.0209548f, -0.0231143f, -0.00121617f, -0.0129815f, -0.0199287f, 0.00863336f, -0.00464991f, 0.0162288f, -0.340115f, -0.011018f, -0.0593997f, 0.00644821f, 0.0416332f, 0.0394596f, 0.0172296f, 0.00494231f, 0.0143805f, -0.00819845f, 0.00196982f, 0.00393258f, 0.0246168f, -0.0235927f, 0.0131416f, -0.0190432f, -0.0237865f, -0.0155627f, 0.0265165f, 0.0162884f, 0.00321098f, 0.0136674f, -0.000966112f, -0.0100813f, -0.00604589f, 0.00889466f, 0.0113945f, 0.0264707f, 0.00371883f, -0.00843358f, 0.0145675f, 0.0048638f, 0.00110399f, -0.00130233f, 0.00740726f, -0.00393368f, -0.0242178f, 0.00341681f, 0.00115369f, -0.00297881f, -0.0844071f, 0.0537151f, -0.00209399f, 0.0310295f, 0.0383914f, 0.00456459f, 0.0188114f, -0.0177144f, 0.0133258f, 0.0584683f, -0.00640495f, 0.0175946f, 0.0186782f, 0.00213311f, 0.00393403f, 0.00382759f, 0.00267507f, 0.00493673f, -0.00856695f, -0.00627955f, -0.0103436f, -0.000671664f, -0.110419f, 0.0307264f, 0.0042176f, 0.0031638f, 0.0154172f, 0.00265482f, 0.0410853f, 0.00833895f, -0.0183989f, -0.000717906f, -0.0090387f, -0.00404523f, -0.00976238f, -0.0137555f, 0.000157289f, -0.00341186f, -0.0214878f, 0.0142639f, 0.00624623f, 0.000537292f, -0.0520912f, -0.0432221f, -0.00330415f, 0.0263942f, -0.00150974f, 0.00172088f, -0.0815726f, -0.0201155f, -0.00986346f, 0.0121252f, 0.00198959f, -0.0349936f, -0.00608366f, -0.00399543f, 0.0192487f, -0.0123156f, 0.0072797f, 0.000507143f, 0.0334805f, 0.000609379f, 0.00961966f, -0.00697663f, 0.00201967f, -0.0207349f, -0.0103385f, -0.00343849f, -0.00330492f, 0.035106f, -0.00456996f, 0.00197528f, 0.016148f, 0.0142903f, 0.0616483f, 0.0093118f, -0.0596028f, 0.00945764f, -0.00659242f, 0.118389f, -0.00259384f, -0.00285344f, 0.00567036f, 0.0195813f, -0.00461807f, -0.0608699f, 0.00380259f, 0.00143385f, -0.00466997f, 0.0194046f, -0.0198423f, -0.00334569f, -0.014399f, 0.0130021f, -0.0141619f, -0.00859914f, 0.00997122f, -0.0198446f, -0.0094162f, -0.0116609f, -0.0111888f, -0.00903524f, 0.00937981f, 0.01772f, -0.00236374f, -0.00870162f, 0.000141193f, -0.0343695f, -0.00997931f, 0.0073531f, -0.100394f, -0.00367661f, -0.00124499f, 0.00318026f, 0.0554203f, -0.00342582f, -0.0104147f, -0.0577869f, -0.0126485f, -0.0332496f, 0.0346141f, 0.0307962f, -0.0174745f, -0.0387339f, 0.0167707f, -0.0363424f, 0.0154902f, -0.0118644f, -4.63543e-06f, -0.0683506f, -0.0344076f, -0.00104884f, -0.00883997f, -0.00305185f, -0.0150299f, -0.0186403f, 0.0110238f, 0.00779224f, -0.0102231f, 0.0087488f, -0.0138988f, -0.0229105f, -0.0244903f, -0.0202919f, 0.00135903f, -0.00574432f, 0.00254918f, 0.0340209f, -0.046428f, -0.00670622f, 0.000925543f, -0.0249251f, -0.00275456f, 0.0199177f, 0.000210993f, 0.027762f, -0.0228046f, 0.0484813f, 0.00538959f, 0.0136714f, -0.00690097f, -0.0448533f, -0.00815204f, 0.00734891f, 0.0173959f, -0.0379109f, 0.0594617f, -0.00722084f, 0.0415935f, 0.014792f, -0.0170252f, -0.0139396f, 0.00146415f, 0.00117702f, 0.0685559f, 0.00727832f, -0.107566f, -0.0112505f, 0.033853f, 0.0046957f, -0.0242369f, 0.0148181f, -0.0723487f, -0.00961667f, 0.0304085f, -0.00520772f, -0.0316467f, 0.0327801f, -0.00755137f, 0.0166041f, -0.0557288f, -0.0227759f, -0.00314548f, 0.0152585f, 0.020071f, -0.0377076f, 0.00687613f, -0.0273935f, -0.00647955f, 0.0105047f, -0.0137238f, 0.023264f, -0.0455722f, -0.00221414f, -0.0258535f, -0.0236395f, 0.0593407f, 0.00448763f, 0.0150777f, 0.00437925f, 0.0295782f, -0.0344752f, 0.00365267f, 0.140464f, -0.0479012f, 0.025726f, 0.119063f, 0.000301925f, -0.00810565f, -0.354073f, -0.0723185f, -0.0046123f, 0.033882f, -0.044552f, -0.0138361f, 0.00384129f, 0.0139111f, -0.01667f, -0.0821503f, 0.0029974f, -0.0306725f, 0.0160366f, 0.0334754f, 0.0192693f, -0.00616713f, -0.00232275f, 0.0107987f, 0.00437057f, 0.0017298f, 0.0196916f, -0.0417255f, -0.00911193f, 0.00876709f, -0.00172422f, -0.00105248f, -0.0191631f, -0.00387423f, -0.0102766f, -0.025317f, -0.0416204f, -0.0319611f, -0.00359193f, 0.00424064f, -0.00575092f, -0.0282402f, 0.0745899f, -0.0126492f, -0.0162564f, -0.261967f, -0.705265f, -0.0403731f, -0.00209634f, -0.694297f, 0.00956909f, 0.0158826f, 0.0130207f, 0.003825f, -0.000300812f, -0.0121346f, 0.00642053f, -0.012902f, 0.0309272f, 0.0609192f, -0.00654145f, -0.0937578f, -0.00432024f, -0.00767539f, 0.0461248f, 0.00701077f, -0.0174477f, 0.00563833f, -0.0107107f, -0.0255275f, 0.00892488f, -0.00166062f, 0.039829f, -0.00150394f, 0.00742194f, -0.00885529f, -0.0103532f, 0.0777858f, 0.0885367f, -0.00425715f, 0.0423651f, -0.0446651f, -0.635069f, -0.00919329f, -0.00356176f, 0.00988705f, 0.0116529f, -0.0401253f, 0.00260105f, 0.00573955f, -0.0667439f, 0.101175f, 0.0765288f, -0.0120077f, 0.00322599f, -0.0192768f, 0.0382749f, -0.222119f, -0.0452036f, 0.0424303f, 0.0890699f, 0.0117557f, 0.0315167f, 0.0284256f, 0.00541845f, -0.250147f, 0.00420668f, -0.0189724f, -0.00416381f, -0.00162803f, -0.0108763f, -0.00970892f, 0.0134476f, -0.0254931f, 0.0307225f, 0.00128596f, 0.0171106f, 0.00467854f, -0.0124376f, 0.0183396f, 0.0021754f, 0.00170886f, -0.0226898f, 0.0250111f, -0.0533301f, -0.0163268f, 0.00618995f, 0.0416378f, 0.0475397f, 0.0105684f, -0.00440933f, 0.0496722f, -0.0215733f, -0.0256361f, -0.0285091f, -0.0276881f, -0.00102202f, -0.0720219f, -0.0296656f,
        0.00465617f, 0.00138814f, -0.0913312f, -0.0161213f, 0.0160887f, 0.0204469f, -0.0223319f, 0.015304f, 0.000397867f, 0.00824013f, 0.0114613f, 0.00408309f, 0.0384456f, -0.00453968f, 0.0176576f, 0.100434f, -0.0393971f, 0.0160015f, -0.00313166f, -0.0058054f, 0.0342083f, 0.0333727f, 0.00275399f, -0.0111393f, -0.0656798f, 0.0117794f, 0.00399766f, 0.00310487f, 0.00290905f, 0.00311256f, 0.0103328f, 0.00221549f, -0.00340486f, -0.00955604f, -0.010614f, 0.0144013f, -0.0244803f, 0.246714f, 0.00585756f, -0.0183366f, 0.0131221f, -0.015529f, 0.0634503f, -0.00107566f, 0.0230663f, -0.00523926f, -0.0100486f, -0.0270644f, 0.0938544f, -0.0136558f, 0.0164469f, -0.349288f, 0.0108305f, 0.0621752f, -0.00813808f, -0.0218271f, 0.0168811f, -0.00509217f, -0.0249135f, 0.0268669f, -0.0294336f, 0.0396944f, -0.00419361f, 0.00843219f, -0.000475472f, -0.0122415f, 0.0142385f, 0.0240099f, -0.0041296f, 0.0167314f, -0.0210217f, -0.00275032f, 0.0121842f, -0.00556776f, -0.0215306f, 0.0411878f, -0.00102203f, 0.00011487f, -0.0142263f, -0.00257424f, -0.0044306f, 0.0115836f, -0.0331884f, 0.0153153f, 0.0023461f, -0.0229996f, -0.00982945f, 0.0207273f, 0.0039542f, -0.0275622f, -0.00118208f, -0.00703868f, -0.0111554f, 0.0155981f, -0.0197133f, -0.00157645f, 0.0790344f, 0.0277319f, -0.0239723f, 0.0133704f, 0.0153687f, -0.0220235f, -0.0652554f, 0.0340702f, -0.0256995f, 0.00463251f, -0.134567f, 0.0048301f, -0.0935251f, -0.0125128f, -0.0560035f, -0.000903825f, 0.0231884f, 0.0678238f, 0.0172834f, 0.0226948f, -0.00784814f, -0.000168366f, 0.0165854f, 0.00979108f, -0.010978f, -0.147669f, 0.020833f, -0.0320907f, -0.339001f, -0.0307849f, -0.00796792f, 0.00704321f, -0.0258511f, 0.0302859f, -0.0174755f, -0.0208662f, -0.00800382f, -0.00772683f, 0.00787931f, 0.0244046f, 0.0635711f, -0.0490687f, 0.00843431f, -0.00969577f, -0.00403176f, -0.00225678f, -0.00425568f, 0.00423476f, -0.0522863f, 0.00901175f, 0.00701737f, 0.0203201f, 0.00764967f, -0.0128627f, -0.0154611f, -0.00973917f, 0.0172989f, 0.00679487f, -0.00897315f, -0.00337138f, -0.0103584f, -0.00507785f, -0.00390477f, 0.0472275f, 0.0060846f, 0.0151745f, 0.0472687f, 0.000490868f, 0.0196255f, 0.00541134f, -0.0206129f, -0.00112977f, -0.0197924f, -0.0553976f, -0.098063f, 0.0664134f, 0.00349375f, 0.00311233f, 0.0401445f, 0.0128354f, -0.0250036f, 0.0436594f, -0.0462325f, -0.00102946f, -0.013474f, -0.0172785f, 0.0394013f, -0.00569089f, 0.000160535f, 0.000504291f, 0.0504433f, -0.0205918f, 0.0101148f, -0.00946464f, -0.0885629f, -0.04032f, -0.012075f, 0.492342f, -0.000999111f, 0.00407901f, 0.0888248f, 0.0100317f, -0.024372f, -0.0211601f, 0.000658811f, -0.0209988f, -0.0190039f, -0.0219266f, -0.0516314f, -0.00642571f, 0.00488745f, 0.00512097f, 0.0145898f, -0.00157307f, 0.0026168f, 0.0156606f, -0.00531944f, -0.017507f, -0.0180003f, 0.00282254f, 0.0143295f, 0.0777137f, -0.00385748f, -0.00549398f, -0.0172826f, 0.0323722f, 0.185825f, 0.0121615f, 0.00399867f, -0.0541097f, 0.0386216f, 0.0595922f, 0.594257f, -0.00955271f, 0.00343269f, 0.0139925f, 0.00328999f, -0.0792421f, -0.045498f, 0.0113837f, -0.00976291f, 0.00624078f, -0.0254107f, -0.0216194f, -0.028773f, 0.0236943f, 0.0197444f, -0.00939094f, 0.0135671f, -0.0407697f, 0.00794318f, -0.0184558f, -0.0282076f, -0.0112124f, 0.00710705f, 0.0203747f, -0.00201855f, -0.0137849f, -0.00224183f, -0.00758043f, 0.0109492f, 0.0111736f, -0.0524165f, -0.00359813f, -0.0105491f, 0.00795013f, 0.0490089f, -0.0172285f, -0.131601f, -0.640844f, -0.00210558f, -0.0191391f, 0.144537f, -0.0187546f, -0.0117677f, -0.0243942f, -0.0673674f, 0.0116665f, -0.00634048f, -0.0171121f, -0.018849f, -0.0452217f, -0.0314511f, 0.01823f, -0.0338747f, -0.00232084f, -0.0184449f, -0.0628265f, -0.00846206f, 0.00285066f, 0.281056f, -0.0109403f, -0.036282f, 0.00725135f, -0.027479f, -0.0120889f, 0.0185699f, -0.00228023f, 0.000971992f, 0.020036f, -0.0437852f, -0.013831f, 0.0284799f, -0.0116033f, -0.0213317f, -0.0391473f, -0.0180216f, 0.0224665f, 0.00661723f, 0.0188164f, -0.00856477f, -0.0188785f, -0.0419517f, -0.0383142f, 0.00822795f, -0.0210551f, 0.0376673f, -0.0158509f, 0.0531296f, -0.0222652f, 0.0202294f, 0.0377989f, -0.0486931f, -0.0236611f, -0.0364076f, -0.0364403f, 0.105507f, -0.0520728f, -0.085646f, -0.0517868f, 0.00898522f, 0.0145328f, -0.0152412f, 0.00230019f, -0.0490983f, 0.0199105f, 0.193699f, -0.00652485f, -0.0293521f, -0.101157f, 0.00759732f, 0.0611226f, 0.00668415f, -0.0644944f, -0.00138395f, -0.0872389f, -0.0289147f, -0.0104552f, 0.0102965f, -0.00918203f, -0.0163947f, 0.00688836f, -0.0460991f, 0.0010579f, -0.0220147f, 0.00389295f, -0.0450669f, -0.0338309f, -0.00643917f, -0.164896f, 0.00520622f, -0.00943891f, 0.015696f, -0.0488516f, 0.00357405f, 0.395393f, 0.0142406f, 0.0375136f, 0.0266987f, 0.00442581f, -0.0355697f, 0.0566785f, -0.0609618f, 0.0953531f, 0.0234361f, -0.0235014f, -0.0201052f, 0.0185904f, 0.0944014f, -0.00254259f, 0.0149094f, -0.00267577f, -0.0236442f, 0.0304207f, 0.0195184f, 0.00453831f, -0.010829f, -0.00384567f, -0.00720987f, 0.00142745f, 0.00339592f, 0.0255406f, -0.0328377f, -0.0418446f, 0.00524565f, -0.019943f, -0.00744414f, -0.0262656f, -0.00295384f, -0.012041f, 0.00168772f, -0.0393009f, -0.0333347f, -0.0127033f, -0.0399219f, -0.12722f, -0.223577f, 0.0811929f, -0.130626f, -0.0705225f, 0.174048f, 0.0435034f, -0.136602f, 0.00640297f, -0.166342f, 0.0597288f, 0.0182928f, 0.00638083f, 0.00566142f, 0.0143743f, -0.0117229f, -0.00092003f, -0.00302193f, 0.0193828f, 0.0549159f, -0.01403f, -0.0686686f, -0.00131562f, -0.0395576f, 0.0140634f, 0.00728921f, -0.0222314f, 0.0847774f, 0.00397858f, -0.037106f, 0.00703206f, 0.0217107f, 0.026982f, -0.0970178f, 0.00170535f, 0.00461989f, -0.0484043f, 0.0549405f, -0.00663961f, -0.0301618f, 0.0402775f, -0.126174f, 0.042974f, 0.00767555f, -0.0323881f, -0.0021808f, 0.00152122f, -0.0794255f, 0.00950137f, 0.00617034f, -0.186531f, 0.0667047f, 0.158624f, -0.0498641f, 0.000181888f, -0.00194408f, 0.0130678f, -0.0624929f, 0.099144f, 0.00810417f, 0.174436f, 0.0147924f, 0.00815054f, 0.0152255f, -0.0833151f, -0.072767f, -0.201512f, -0.0109339f, -0.003133f, -0.00430304f, -0.0208616f, -0.0187232f, 0.0277294f, -0.451013f, 0.0336152f, -0.00462652f, 0.00806012f, -0.000483294f, 0.0313363f, 0.0948398f, -0.0302999f, -0.00779582f, -0.0975373f, 0.0429978f, -0.0117262f, -0.00451523f, -0.0175741f, 0.0914118f, 0.0390275f, 0.00306197f, 0.0172763f, 0.0486995f, -0.0628708f, -0.00845093f, 0.00565009f, -0.0126375f, 0.0362389f, -0.0893211f, -0.0264466f,
        0.0309426f, -0.0247239f, -0.0618656f, -0.16444f, 0.0416493f, -0.0039234f, -0.0446445f, -0.0806408f, 0.0315374f, -0.0123988f, 0.0385759f, 0.0315165f, 0.00742563f, -0.0276244f, 0.013597f, -0.000546713f, -0.126003f, -0.0403999f, -0.0199147f, 0.090123f, 0.0122743f, 0.0904552f, 0.0480448f, -0.0274991f, -0.0463688f, 0.132874f, -0.0163207f, 0.00931698f, 0.00050237f, -0.034227f, 0.0273549f, 0.0257694f, 0.0545361f, -0.0196519f, -0.00616926f, 0.0252382f, 0.00394299f, 0.00503618f, -0.000107687f, -0.00739968f, 0.0155088f, -0.0271828f, 0.0136159f, -0.0184294f, 0.00419291f, -0.0705982f, 0.00832841f, -0.0455188f, 0.0203078f, -0.0104058f, -0.00448528f, 0.0346675f, 0.00227903f, 0.0283768f, 0.0146701f, 0.0238016f, -0.0041065f, -0.00951874f, -0.0656203f, 0.00289312f, -0.0280637f, 0.064775f, -0.0145084f, -0.0166982f, 0.112919f, -0.030709f, -0.08767f, 0.0231176f, -0.00683745f, 0.145201f, -0.0588483f, -0.00211676f, 0.0707442f, -0.0175353f, 0.0425204f, 0.047214f, -0.00454212f, 0.108341f, -0.0655429f, -0.0661698f, -0.00742549f, 0.0525604f, -0.00200138f, 0.0760939f, 0.0208251f, -0.0183413f, -0.019956f, 0.0497461f, -0.00312012f, -0.026077f, -0.00492334f, -0.0389153f, -0.0240003f, -0.0236527f, -0.00949685f, 0.00834218f, 0.196113f, -0.0203076f, -0.0373067f, 0.0511745f, -0.000502779f, -0.0506356f, 0.0270005f, 0.0560514f, -0.0566957f, 0.00592365f, -0.0950855f, 0.0330845f, 0.0126008f, -0.0178738f, 0.00655207f, -0.00560155f, 0.0226922f, 0.122885f, -0.0227311f, -0.0185407f, -0.024025f, 0.000734875f, -0.0501656f, 0.00259467f, -0.0401208f, -0.00270448f, 0.0298842f, -0.0449168f, -0.083653f, -0.0667249f, -0.012424f, 0.0228182f, -0.0256871f, 0.0103425f, 0.00584589f, -0.0313978f, -0.00512387f, -0.0389378f, 0.00783504f, 0.0246462f, 0.0204282f, -0.0313174f, 0.0293227f, -0.0135298f, 0.0250816f, 0.00154453f, 0.00455047f, 0.0664336f, -0.0924272f, 0.0141598f, 0.0249505f, 0.0114919f, 0.127537f, -0.0302333f, -0.0464173f, 0.0312457f, 0.0119746f, 0.00862732f, -0.0221585f, -0.00284848f, 0.014157f, 0.0253277f, 0.00495452f, 0.00886403f, 0.00389645f, -0.0347684f, -0.0039163f, 0.0218669f, -0.0417104f, 0.00547612f, -0.013528f, -0.00265715f, 0.180858f, -0.000752272f, -0.18944f, 0.0260848f, -0.000632882f, 0.0126054f, 0.0359676f, 0.0302849f, -0.0371376f, 0.0941217f, -0.0281283f, -0.0280773f, -0.011986f, -0.0406752f, 0.239648f, -0.00517518f, -0.00410975f, 0.00103368f, 0.0209206f, -0.0476301f, 0.00454544f, -0.0149667f, -0.0314583f, -0.00242636f, -0.0512553f, 0.0608112f, 0.0428258f, 0.0173526f, 0.0602241f, -0.0548611f, -0.131965f, -0.0495486f, 0.00765915f, -0.062264f, -0.000979455f, -0.0652348f, -0.147691f, -0.0231597f, 0.0251012f, -0.0946399f, 0.0277068f, -0.00621829f, 0.0313192f, 0.0259072f, 0.00394534f, -0.0118648f, 0.004981f, 0.0594206f, -0.0358001f, -0.0710233f, -0.00969833f, 0.023656f, -0.0388052f, -0.00855584f, 0.259141f, 0.0142973f, -0.00158563f, 0.0164536f, 0.0212657f, 0.00174633f, 0.0514006f, -0.00881672f, 0.0221807f, 0.0413859f, 0.0143335f, -0.163744f, 0.236609f, 0.0189168f, -0.0167902f, 0.0688642f, -0.0370002f, -0.0330411f, -0.0653769f, 0.00270779f, -0.00759605f, -0.0221796f, 0.0385442f, -0.0446415f, 0.06948f, -0.033133f, -0.0352207f, -0.0310347f, 0.00721417f, 0.0857527f, 0.00283876f, -0.115239f, 0.0347347f, -0.0365242f, 0.0587821f, 0.00664576f, -0.0273541f, -0.016766f, -0.0138301f, 0.00564337f, 0.0364023f, -0.0560315f, -0.0449002f, -0.0932135f, -0.0177926f, -0.0494535f, 0.0610707f, -0.00528969f, 0.114377f, -0.0275389f, 0.0177389f, -0.0280061f, -0.00589614f, -0.00858413f, 0.0105453f, -0.0247948f, -0.0472122f, -0.000931705f, -0.0574841f, -0.0412944f, 0.00216405f, -0.0681429f, -0.00229429f, 0.00222781f, -0.0102497f, -0.0110639f, 0.0254925f, 0.0135797f, -0.0289002f, 0.00603638f, 0.0356664f, -0.0870163f, 0.552476f, 0.0106117f, -0.025193f, -0.0567232f, 0.00731144f, -0.00597211f, 0.00564131f, -0.037914f, 0.00553956f, 0.0244306f, 0.0163081f, 0.0614898f, -0.0103462f, -0.0125773f, -0.0129543f, -0.0425792f, -0.00984468f, 0.0241087f, 0.0391885f, 0.0113726f, 0.0740247f, -0.0314575f, 0.0847706f, 0.00766129f, -0.00782563f, -0.00219977f, 0.0364213f, 0.00561357f, -0.0207095f, -0.0389947f, -0.0574235f, -0.0215928f, 0.0242519f, 0.0150763f, 0.00640004f, 0.0049859f, -0.0883498f, 0.0259088f, -0.00976872f, -0.0257561f, -0.145433f, 0.0186583f, -0.0313577f, 0.0232484f, 0.135472f, -0.0611472f, -0.0134871f, -0.0152308f, 0.0481365f, -0.000509527f, 0.0241717f, -0.0205968f, -0.0464828f, 0.00742741f, -0.0585818f, 0.0174123f, -0.032865f, 0.0399474f, 0.0189778f, 0.0185407f, -0.0144228f, 0.0195944f, 0.0105867f, 0.0108527f, 0.0318328f, -0.07468f, 0.0640258f, -0.0166149f, -0.0161666f, 0.0270572f, -0.00831346f, 0.0213354f, -0.0331297f, 0.0314013f, -0.0295451f, -0.0309544f, 0.00883464f, -0.000784053f, 0.00228157f, 0.030596f, -0.0169894f, -0.0723077f, 0.0142356f, -0.042197f, -0.0273198f, 0.0607149f, 0.0824823f, 0.0722077f, -0.0207748f, -0.0090944f, 0.0268541f, 0.0273479f, 0.00481306f, -0.00487477f, -0.0183224f, -0.0126787f, 0.0311318f, -0.0985153f, -0.0152497f, 0.00489618f, -0.0141078f, -0.0060658f, -0.000568589f, -0.032613f, 0.00976906f, -0.0462634f, -0.0259696f, -0.0786609f, -0.0153404f, 0.0249492f, 0.00292531f, -0.0255124f, 0.0202219f, 0.0304817f, -0.0177191f, -0.0135411f, -0.0064023f, 0.048916f, 0.0348483f, -0.00747575f, 0.0256531f, -0.0264167f, -0.027836f, 0.026632f, -0.0408624f, 0.0405082f, 0.0435032f, -0.0481381f, 0.0232822f, 0.0406269f, -0.104934f, 0.032984f, 0.00642478f, -0.0123055f, 0.0323379f, 0.0262914f, -0.00313157f, -0.0307961f, -0.059502f, 0.043095f, -0.0842975f, -0.0634201f, -0.0069968f, -0.0269704f, 0.0525556f, -0.0145985f, -0.026517f, 0.0287775f, -0.00225143f, 0.00998218f, -0.0208695f, 0.00038333f, -0.0179813f, 0.0299511f, -0.0270286f, -0.0215702f, 0.00986492f, -0.121571f, 0.0374826f, 0.0280122f, -0.0349332f, 0.00798409f, 0.00126605f, 0.0544963f, -0.00189064f, -0.0770879f, -0.00792704f, 0.0613617f, 0.0133352f, 0.0303873f, -0.000380032f, 0.0189077f, -0.0194632f, -0.00659714f, -0.0571043f, 0.041608f, -0.0141942f, 0.012823f, 0.00537086f, 0.000970999f, 0.0332154f, 0.0570762f, -0.0137126f, 0.0101087f, -0.00108052f, -0.0265809f, -0.0247709f, -0.00362676f, -0.0148946f, 0.013131f, -0.00308769f, -0.158096f, 0.00257066f, -0.0143705f, 0.0888035f, 0.00916709f, 0.00514034f, -0.0227268f, 0.134988f, -0.0492885f, 0.0022784f, -0.0144922f, 0.0256463f, 0.0246127f, -0.0242015f, -0.0270194f,
        0.0236487f, -0.00133765f, -0.023996f, 0.0121123f, 0.0473768f, -0.0229827f, 0.0620781f, 0.0348273f, 0.0118778f, -0.0358558f, -0.00418959f, 0.026328f, 0.00159447f, -0.0285201f, 0.0242085f, 0.024281f, -0.120022f, 0.00322402f, -0.00124464f, -0.00395719f, 0.00586048f, 0.0264264f, 0.0202582f, -0.0172882f, 0.0167585f, 0.00926656f, 0.00103096f, 0.00249462f, 0.00288184f, -0.00771514f, 0.0255329f, 0.0516628f, -0.0170072f, -0.00388561f, -0.00997277f, 0.0355019f, 0.000978238f, -0.144348f, -0.00646585f, -0.013882f, 0.033804f, -0.0377087f, 0.00771159f, -0.0061665f, 0.0237085f, -0.0122598f, 0.0771705f, -0.0542605f, -0.0292168f, -0.0110855f, 0.00780249f, -0.0262439f, -0.0170252f, 0.0232333f, 0.0221474f, -0.000682905f, 0.0456239f, 0.00516233f, -0.0356498f, 0.0433573f, -0.0725911f, 0.122393f, -0.000836771f, 0.0154195f, -0.00217232f, -0.0458872f, 0.0576701f, 0.0347757f, 0.00437707f, 0.0167836f, -0.024089f, 0.00395376f, 0.0226754f, -0.000325613f, -0.0119747f, 0.0166885f, 0.0133881f, -0.00825686f, -0.0115485f, -0.0256805f, -0.013069f, 0.029991f, -0.0104672f, 0.0468771f, 0.018202f, -0.0499781f, -0.0150365f, 0.0351706f, 0.000881884f, 0.0257364f, -0.00567146f, -0.0125245f, -0.00638529f, 0.00949407f, -0.00206895f, -0.00294736f, -0.00599403f, 0.0100478f, -0.0708312f, 0.0164853f, -0.00509979f, -0.0820398f, 0.00301894f, -0.011352f, -0.103304f, 0.0361376f, -0.00276168f, 0.0140668f, 0.0182486f, -0.0224722f, 0.00670642f, -0.00173934f, -0.0763404f, 0.00545386f, -0.0451032f, 0.258199f, -0.000526159f, -0.00244376f, -0.0070213f, 0.0136966f, 0.00651444f, 0.00336226f, 0.0129456f, -0.00535145f, -0.0337439f, -0.0488545f, 0.0363396f, -0.000131419f, -0.0442874f, -0.00468587f, -0.00406768f, -0.0170205f, -0.0192772f, -0.00277597f, 0.0212662f, 0.0767458f, -0.0198272f, 0.00671115f, 0.00387314f, -0.00222632f, 0.017668f, -0.0152864f, -0.00217823f, -0.0302261f, 0.0201784f, 0.00912841f, 0.0418803f, 0.00397826f, -0.0171634f, 0.0562426f, -0.00595202f, 0.0317872f, 0.00277863f, -0.0198806f, -0.0105047f, -0.0078311f, -0.00416702f, 0.0284072f, 0.00135271f, 0.00845078f, 0.0125683f, -0.00724979f, 0.0567957f, 0.0255109f, 0.002417f, 0.0114722f, -0.0229208f, 0.00542141f, 0.000680912f, -0.0124263f, -0.0973681f, 0.0429572f, -0.00896565f, 0.00102447f, 0.0209145f, 0.0365617f, 0.00698999f, 0.0611891f, -0.0021814f, -0.00791606f, 0.0636013f, -0.0503155f, 0.041678f, -0.00722059f, -0.00547887f, 0.00243705f, -0.0177814f, -0.12321f, 0.0569086f, -0.00487058f, 0.0123446f, 0.0015868f, -0.0272469f, 0.0180903f, 0.0104843f, 0.0105209f, 0.00808024f, -0.0662313f, -0.0499085f, -0.0297908f, 0.00678693f, 0.0158422f, -0.0149847f, -0.212685f, -0.029142f, -0.0216139f, 0.0197027f, -0.00509483f, 0.0406666f, -0.00101148f, 0.0137954f, 0.0292058f, 0.0261623f, 0.0879647f, -0.0120199f, 0.0276628f, -0.00208332f, 0.00630364f, -0.00283301f, 0.0313885f, 0.00132789f, 0.00430711f, 0.131565f, 0.00856252f, -0.0451589f, 0.0151607f, -0.00609563f, 0.104563f, 0.0503204f, -0.00188153f, -0.00152094f, 0.0331939f, -0.0268272f, -0.0720271f, 0.0120254f, 0.00428272f, -0.010781f, -0.0235618f, -0.0599427f, -0.0128298f, -0.039684f, 0.0124311f, -0.00907946f, -0.0219339f, -0.00574204f, 0.00290369f, -0.0397143f, -0.0306637f, 0.0046412f, -0.102802f, 0.02052f, 0.0177221f, -0.000307451f, -0.663219f, -0.00099111f, -0.00863413f, -0.0648291f, 0.141571f, -0.0264896f, -0.00967159f, -0.0105556f, 0.00667919f, 0.019933f, -0.0081883f, -0.0256497f, -0.0425081f, -0.00260382f, -0.00437219f, 0.0181059f, 0.0588014f, -0.0156841f, -0.0992774f, 0.0577409f, -0.0112435f, 0.0118955f, -0.01259f, -1.68039e-05f, -0.0231843f, -0.0715207f, 0.00562568f, 0.00659099f, -0.00432696f, 0.0402245f, -0.0132643f, 8.8306e-05f, 0.00698941f, -0.0695019f, -0.0112349f, 0.0696259f, -0.142201f, -0.0227633f, -0.019462f, -0.0518398f, -0.0213576f, 0.0148991f, 0.0344155f, -0.0131575f, -0.012708f, -0.00177817f, -0.00639755f, -0.000887201f, -0.0257106f, -0.0247181f, 0.00548285f, 0.0290425f, 0.122557f, -0.00347772f, 0.0268244f, -0.00612725f, -0.0196236f, -0.0472946f, 0.00890478f, 0.000844572f, 0.0154442f, 0.024701f, -0.0306896f, 0.0231992f, 0.0425512f, -0.0302086f, 0.0319046f, 0.0310391f, -0.00796268f, -0.0411025f, 0.00749199f, -0.0374908f, -0.0108962f, 0.0293042f, 0.00369268f, -0.0138972f, -0.00285899f, -0.0473339f, 0.00105261f, 0.0269907f, -0.0314717f, -0.0538936f, 0.0837861f, -0.0145771f, 0.0345362f, 0.222726f, -0.034146f, -0.0154113f, 0.0519213f, 0.0351403f, -0.0609869f, 0.0181544f, -0.0165051f, 0.00702428f, -0.0109979f, -0.00444243f, -0.018915f, -0.027162f, 0.00253407f, 0.0133815f, -0.000469394f, 0.109107f, 0.0153356f, 0.00683112f, 0.0128685f, 0.0282692f, -0.0384653f, 0.000389417f, 0.106818f, 0.0799349f, 0.0567321f, 0.0479257f, 0.00394279f, -0.00575818f, -0.575371f, -0.0118667f, 0.00356253f, -0.0399865f, -0.0217626f, -0.019511f, 0.0108772f, 0.0134627f, -0.000487889f, -0.00162015f, -0.0268957f, 0.0158162f, 0.0124589f, 0.0514896f, 0.0391116f, -0.02102f, 0.0289451f, -0.0162062f, 0.0295524f, 0.0240599f, 0.00653552f, -0.0296798f, -0.0614426f, 0.00678693f, -0.0126935f, -0.0259306f, -0.0270236f, -0.005202f, -0.027559f, -0.00571665f, 0.01303f, -0.0176816f, 0.00828625f, -0.0159388f, 0.016197f, -0.0685197f, 0.0359586f, -0.0149305f, -0.0100357f, -0.054005f, 0.0405895f, -0.0436483f, -0.0196033f, 0.0205626f, 0.0601753f, 0.00745636f, 0.00526461f, 0.00770411f, -0.00536197f, -0.0196271f, -0.00742883f, 0.0673765f, 0.0225239f, 0.0330661f, -0.0197954f, 0.0635232f, -0.00196483f, -0.0160432f, 0.0274051f, 0.0249642f, -0.0215083f, 0.00376016f, 0.0484418f, -0.0339058f, -0.00930553f, 0.000391001f, 0.0489547f, 0.00680175f, 0.0121302f, -0.0159317f, -0.00746274f, 0.00762586f, 0.0151285f, -0.00984925f, 0.00967698f, -0.063813f, -0.00191317f, -0.0225768f, -0.0460198f, 0.0129389f, 0.022693f, -0.0331679f, -0.0252172f, 0.0152612f, -0.0615063f, 0.00776267f, 0.0890267f, -0.0218608f, 0.0164835f, -0.048754f, 0.0158734f, 0.00247796f, -0.0340838f, 0.0199824f, 0.0422744f, 0.00495236f, 0.00733676f, -0.693422f, -0.057195f, -0.042145f, -0.0894016f, 0.00573138f, 0.00168211f, -0.00815092f, 0.1004f, -0.00830388f, 0.0212194f, 0.00796229f, 0.0182782f, -0.00677567f, -0.0025772f, -0.0141583f, -0.0503938f, 0.00933939f, -0.0440368f, -0.0650577f, -0.0133163f, -0.0150479f, -0.128004f, -0.025883f, -0.0142512f, 0.0267747f, 0.0603829f, 0.0616747f, 0.00518816f, 0.0353825f, -0.0136665f, -0.0116953f, -0.0117363f, -0.00988685f, 0.0161024f, -0.0164802f, 0.0120735f,
        0.0115264f, 0.00956785f, -0.0348965f, -0.0115787f, 0.0441999f, 0.0345045f, 0.0134386f, -0.0337335f, -0.00245127f, -0.0610053f, 0.0043896f, 0.0019506f, 0.013525f, -0.0545739f, 0.0306072f, 0.105704f, -0.0610636f, 0.0184838f, -0.0121108f, -0.00898275f, 0.0264786f, 0.0351719f, 0.00565877f, -0.00984551f, 0.0349376f, 0.0065558f, 0.000771663f, 0.000747164f, 0.00623147f, -0.0100182f, 0.0147877f, 0.027002f, -0.0082708f, -0.00312388f, -0.031057f, 0.0352335f, 0.0102762f, -0.136548f, -0.00137814f, -0.0245331f, 0.0302073f, -0.050357f, -0.0055813f, -0.0035066f, 0.0159663f, -0.00413293f, -0.0220518f, -0.0378098f, -0.000528503f, -0.00883574f, -0.0160642f, -0.0976056f, -0.00949359f, 0.0667935f, 0.0152671f, -0.00275173f, -0.00305567f, -0.00027522f, -0.0358676f, 0.0613587f, -0.0621408f, 0.0603126f, -0.00382261f, -0.0162797f, 0.0627967f, -0.0338104f, 0.019684f, 0.0723154f, 0.0405459f, 0.0150282f, 0.0116941f, 0.0159087f, 0.0423308f, 0.000188638f, -0.0151563f, 0.0213552f, 0.0260785f, -0.000634076f, -0.00666879f, -0.0143571f, -0.0154005f, 0.0452614f, -0.0241995f, 0.00760913f, 0.00565907f, -0.0146403f, -0.00882357f, 0.109466f, 0.000185842f, 0.0530813f, -0.0167083f, -0.0132453f, 0.00510363f, 0.000928611f, -0.0231941f, -0.00849421f, -0.0127253f, 0.0143131f, -0.104331f, 0.0150856f, -0.0115339f, -0.0400927f, -0.00650179f, 0.00782663f, -0.0161432f, 0.00612369f, -0.0368485f, 0.0320765f, -0.000285285f, -0.0252538f, 0.00567933f, -0.00326235f, -0.0118118f, -0.0067807f, -0.0626707f, 0.0314245f, -0.00367115f, 0.0034559f, 0.00094028f, 0.012767f, -0.0376215f, -0.0102952f, 0.0236869f, 0.00184345f, -0.0418395f, -0.0542331f, -0.00655869f, -0.00491183f, -0.0167015f, -0.0135059f, -0.0126727f, -0.0262544f, -0.0235505f, -0.00927455f, 0.044421f, 0.0340354f, 0.0544527f, 0.0133111f, 0.00308665f, 0.00078136f, -0.0023735f, -0.0141342f, 0.00124783f, -0.0175074f, 0.0506524f, 0.0344784f, 0.016513f, 0.00434411f, -0.0224391f, 0.0865785f, -0.00372209f, -0.0103298f, -0.00164323f, -0.0143697f, -0.0125625f, -0.00602005f, -0.00435671f, -0.0097799f, -0.00277924f, 0.0124438f, 0.00866435f, 0.00456806f, 0.032294f, 0.00501145f, 0.0381001f, 0.0142146f, -0.0373586f, -0.0278584f, -0.0268059f, -0.0109542f, 0.0129881f, -0.0289077f, -0.00849425f, 0.00391238f, 0.0105073f, 0.0449334f, 0.00855353f, 0.0402285f, -0.00646413f, -0.00671409f, 0.013527f, -0.0528845f, 0.0319318f, -0.0113917f, -0.0113392f, -0.000316065f, 0.0412851f, -0.0162739f, 0.0137208f, -0.0163712f, 0.0349673f, 0.00457418f, -0.0198638f, 0.0765183f, -0.001026f, 0.0113388f, 0.00846672f, 0.0122229f, -0.0401006f, -0.00219702f, 0.00703645f, 0.0321573f, 0.000362714f, -0.24312f, -0.014646f, -0.00614563f, 0.0187569f, -0.00394876f, 0.0243838f, -0.00188284f, 0.0050112f, 0.0221267f, -0.00302741f, 0.0435336f, -0.0226377f, 0.0262879f, 0.0155468f, 0.0279725f, -0.00188527f, -0.00564561f, -0.00020769f, 0.0150204f, 0.13116f, 0.021348f, 0.00731956f, -0.0343524f, 0.00212442f, 0.0352829f, 0.526485f, -0.00325235f, -0.00250349f, 0.0161844f, -0.0453576f, -0.0154224f, -0.0407768f, 0.0031079f, -0.00879997f, 0.00831367f, -0.0461003f, -0.0249753f, -0.0173187f, 0.0510597f, 0.0221946f, -0.0149577f, 0.000957178f, 0.0111411f, 0.00876051f, -0.0220329f, -0.0046637f, -0.020372f, 0.00369127f, 0.039286f, -0.00385722f, 0.0115072f, -0.00474474f, -0.0141273f, -0.19162f, -0.0187427f, -0.00145075f, -0.00458649f, -0.00136821f, 0.0037382f, 0.0102019f, -0.0101349f, -0.0303892f, -0.697959f, -0.00391341f, -0.00169856f, 0.0454146f, -0.0300301f, -0.0387779f, -0.0249505f, -0.0183996f, -0.00471838f, -0.00533851f, 0.000305908f, -0.00737827f, -0.0143906f, -0.0612462f, 0.0117793f, -0.0296389f, -0.0045701f, 0.0974987f, -0.0222056f, -0.00917552f, 0.00540695f, 0.376f, -0.0369584f, 0.0818413f, -0.0806179f, -0.0591828f, -0.0292424f, 0.0175326f, -0.0141385f, 0.01833f, 0.0209717f, -0.0198613f, -0.0303378f, -0.00184021f, -0.095508f, 0.00121903f, 0.00795399f, -0.0660669f, -0.000692821f, 0.00370955f, 0.140168f, -0.000690335f, 0.0085036f, -0.0224978f, 0.0989872f, -0.103726f, -0.00133824f, 0.00176511f, 0.0226218f, 0.00723803f, -0.0136401f, 0.0136266f, 0.00908615f, -0.0421018f, -0.0535609f, -0.0230947f, -0.0338358f, -0.00108633f, -0.0356084f, -0.109221f, -0.014515f, 0.0077523f, 0.0139792f, -0.0248496f, -0.023008f, -0.0472426f, 0.0865438f, 0.000595621f, -0.0451802f, -0.0395005f, 0.0493621f, -0.00124904f, 0.0988936f, 0.0572095f, -0.0729679f, -0.00415711f, 0.161504f, -0.00328739f, -0.0133308f, 0.00799106f, -0.0163052f, -0.0209516f, 0.00308542f, -0.0129289f, -0.0510538f, -0.0122714f, -0.0362058f, 0.0683402f, -0.0126313f, 0.0263825f, 0.0168551f, 0.00470125f, 0.0204198f, 0.0145374f, -0.021401f, 0.00460656f, 0.085484f, 0.0781075f, 0.0251125f, 0.00791536f, -0.0189591f, -0.0431845f, 0.051558f, 0.017842f, 0.36608f, -0.0343333f, -0.0303445f, -0.0115494f, 0.0530173f, 0.0165506f, -0.0235855f, -0.052452f, -0.00888096f, 0.0221193f, 0.0386185f, 0.0353902f, 0.0246971f, -0.0122489f, 0.0512722f, 0.00400143f, 0.0255521f, 0.00548785f, 0.00233302f, -0.0253462f, -0.0966852f, 0.00378993f, 0.00350757f, -0.0310213f, -0.0279353f, -0.00233223f, -0.0220107f, 0.00163079f, -0.00717164f, 0.00659987f, -0.00608499f, -0.02305f, 0.00402512f, -0.32546f, 0.0706807f, 0.0274278f, 0.0267394f, -0.00604822f, 0.0361692f, -0.0515999f, 0.0369351f, 0.0124044f, 0.0716815f, 0.0053833f, 0.00673388f, 0.0250085f, -0.000686182f, -0.00550432f, -0.00231397f, 0.00181825f, 0.022164f, 0.0330005f, -0.00140523f, 0.0463948f, -0.0278037f, -0.0318544f, 0.0275073f, 0.0620945f, -0.0128747f, 0.0329174f, 0.0206743f, -0.0352932f, -0.00835452f, 0.0248623f, 0.119621f, -0.0292978f, -0.0132096f, -0.0302576f, -0.0178306f, 0.0209123f, 0.0229405f, -0.0236861f, 0.00108116f, -0.0799521f, 0.00532662f, 0.0127616f, -0.00190055f, 0.00847102f, 0.00451121f, -0.0637118f, -0.0302129f, 0.0119081f, -0.117328f, -0.00946109f, 0.0605782f, -0.0390624f, 0.0192556f, -0.0170363f, 0.0300991f, 0.0444662f, 0.0422317f, 0.0170539f, 0.0504948f, 0.0270332f, 0.00916911f, 0.0242343f, 0.00898315f, -0.0158267f, -0.0475899f, 0.0175909f, -0.000817633f, -0.0176624f, 0.0975135f, -0.00854145f, 0.0155055f, 0.00762038f, 0.0229743f, -0.0525053f, -0.0149161f, -0.0367894f, -0.104801f, 0.013039f, -0.120883f, -0.0715135f, -0.0193206f, 0.0158965f, -0.0748989f, -0.120509f, -0.0506567f, 0.0147239f, 0.107749f, 0.0659703f, 0.0220761f, 0.0242295f, 0.0180054f, -0.0111281f, -0.0171504f, -0.014431f, 0.083154f, 0.0241038f, 0.0115941f,
        0.0112054f, -0.208447f, -0.0871743f, -0.0362684f, -0.0110118f, 0.068481f, 0.0322887f, -0.0375058f, -0.0130676f, -0.101841f, 0.0479009f, 0.0459907f, 0.00208143f, -0.0880017f, 0.0160549f, -0.0533964f, -0.0336657f, -0.000403741f, 0.0274574f, 0.00649047f, -0.0278283f, -0.0254132f, 0.0467184f, -0.0375531f, 0.127941f, 0.0291329f, 0.00155753f, 0.00199031f, 0.0183402f, 0.155697f, 0.0500429f, 0.00407514f, 0.0229933f, -0.00482785f, -0.0220735f, 0.0390895f, -0.0863406f, -0.132777f, 0.00204372f, -0.0069423f, 0.0260759f, -0.031759f, -0.00107891f, -0.0218382f, 0.00464639f, -0.00370248f, 0.00721869f, -0.0152541f, -0.00113688f, -0.00731756f, -0.0459436f, -0.0122795f, -0.0212339f, 0.072953f, 0.0268922f, -0.00254329f, -0.00535364f, 0.0200235f, -0.019393f, 0.00740422f, -0.0515143f, 0.0410708f, -0.00789718f, -0.0633389f, 0.0544137f, -0.0580859f, 0.0325159f, -0.015541f, 0.0178216f, 0.289658f, -0.0234133f, -0.0074536f, 0.0255261f, 0.00291012f, -0.0219596f, 0.0246941f, -0.00560577f, 0.00899517f, 0.00914874f, -0.0254892f, -0.0521876f, 0.0629406f, -0.00645591f, 0.111561f, 0.0122516f, -0.0106223f, -0.0132192f, -0.0819937f, 0.0132221f, -0.00695472f, -0.0207924f, -0.0723628f, 0.0495887f, -0.0359372f, -0.04756f, -0.0288064f, -0.08486f, 0.285901f, -0.0527237f, 0.0401743f, 0.00317573f, -0.00912604f, -0.00509804f, -0.019646f, -0.0133663f, 0.00250147f, 0.00489291f, 0.017901f, 0.117288f, -0.0253837f, 0.0201622f, -0.0127631f, -0.000326688f, -0.0153231f, -0.0756543f, 0.113002f, -0.0181392f, -0.00927301f, 0.0726324f, 0.00722584f, -0.0730271f, 0.0245927f, -0.102462f, 0.0356965f, -0.0606429f, -0.0444952f, -0.0166311f, 0.00795211f, -0.00189904f, -0.0158499f, -0.0204771f, -0.0472794f, -0.0079858f, -0.0501545f, 0.102751f, 0.0584957f, 0.0372233f, 0.00862791f, 0.00449617f, -0.0237138f, 0.00679621f, -0.0152089f, -0.00387291f, -0.126512f, -0.0284672f, -0.0684034f, 0.0303137f, -0.0162955f, -0.0581197f, -0.220276f, -0.00417518f, -0.0689113f, -0.017655f, -0.0224894f, 0.0357768f, 0.0133865f, 0.022937f, 0.0472434f, -0.00953042f, -0.0159915f, 0.00998823f, 0.00600883f, 0.0533401f, 0.194183f, 0.477756f, 0.0191196f, 0.0227464f, -0.00284643f, -0.13471f, 0.0769816f, 0.01241f, -0.0497929f, -0.0935632f, 0.0292851f, 0.0178327f, 0.104592f, -0.0467304f, -0.00100124f, -0.0401962f, -0.0224538f, -0.00678469f, -0.073481f, 0.227438f, -0.00830996f, 0.073789f, -0.0239749f, 0.154952f, -0.0544236f, 0.0156297f, 0.19281f, 0.0326588f, -0.00926173f, -0.0288493f, 0.0228173f, 0.0186095f, 0.0415022f, 0.0290895f, -0.00247426f, -0.0898812f, 0.0274265f, 0.0393059f, 0.0222607f, 0.019877f, -0.150684f, -0.262853f, -0.0894445f, -0.0205114f, -0.00142168f, 0.126473f, -3.85201e-05f, 0.0356633f, 0.0269576f, 0.0157574f, -0.0432543f, 0.0279592f, 0.024804f, -0.0267448f, 0.0191669f, -0.0040675f, 0.0139007f, 0.00963236f, -0.0110146f, 0.137714f, 0.0166686f, 0.0200946f, -0.0611695f, -0.0639973f, 0.0055134f, 0.042783f, 0.0271225f, -0.0468356f, 0.0247138f, 0.0103724f, 0.00932251f, -0.0140851f, 0.0358128f, -0.0059887f, 0.0386251f, -0.00545864f, 0.0596616f, -0.0379678f, 0.0116168f, -0.0113317f, -0.0299328f, 0.0217457f, 0.0063076f, -0.00526829f, -0.012835f, 0.0163333f, -0.0390477f, 0.0108823f, 0.127479f, -0.00949771f, 0.000669599f, -0.00832522f, -0.00771118f, -0.554012f, -0.259737f, 0.00827122f, -0.000538992f, 0.0152035f, 0.05717f, 0.00494831f, -0.0414577f, 0.0166355f, 0.0400496f, -0.0114314f, -0.0214246f, 0.00867137f, -0.0404191f, -0.0166356f, 0.0428265f, 0.0146152f, 0.00234592f, -0.0864799f, 0.0226774f, 0.00508847f, 0.0203778f, -0.0583453f, -0.00666855f, -0.127756f, -0.00862127f, 0.0452925f, -0.0831513f, -0.00326817f, 0.00995622f, 0.116901f, -0.0877858f, 0.112396f, -0.102312f, -0.105516f, -0.0259396f, 0.00757632f, 0.00122858f, 0.0103624f, 0.0457345f, -0.0242102f, -0.0583132f, -0.012498f, -0.313943f, 0.0069556f, -0.0319396f, 0.0172862f, -0.00853725f, 0.0116005f, 0.125311f, 0.00419865f, 0.0476964f, 0.00896339f, 0.00977134f, -0.0925261f, 0.0156905f, -0.018496f, 0.0196972f, 0.0157389f, -0.00196949f, 0.0145061f, -0.0606428f, -0.0694258f, 0.0709404f, 0.00871243f, 0.00455373f, -0.00558034f, -0.0824924f, 0.0011513f, 0.0384797f, 0.00638306f, 0.00363507f, -0.0606946f, -0.0774373f, -0.020545f, 0.0937525f, -0.00557294f, -0.0987101f, -0.0864387f, -0.0108511f, -0.0149365f, 0.0481765f, 0.036998f, -0.112909f, -0.00983293f, 0.135054f, 0.071086f, 0.019128f, 0.00687174f, -0.00651517f, -0.0349884f, -0.00583317f, -0.0110052f, 0.0398168f, -0.0141334f, -0.0344924f, -0.00134893f, -0.0270122f, -0.000114596f, 0.0220215f, -0.0321631f, 0.0329176f, 0.0261847f, 0.170964f, 0.0083325f, -0.0209986f, -0.00422142f, 0.00124639f, -0.000368193f, 0.00871341f, -0.0488562f, 0.0170233f, 0.0236273f, -0.0163899f, -0.120393f, -0.151225f, -0.00206636f, 0.105974f, -0.00312998f, 0.0290657f, -0.014112f, -0.0107348f, 0.032648f, 0.026346f, 0.0817057f, 0.0319139f, 0.00208954f, 0.0860523f, 0.0385837f, 0.115668f, 0.0399777f, 0.00339539f, -0.00545459f, 0.0598012f, 0.00298756f, 0.0148942f, -0.0227489f, -0.0139737f, -0.00473305f, -0.0326132f, -0.0229166f, 0.0207593f, 0.00277288f, -0.00719371f, -0.0202886f, -0.00475025f, 0.00617697f, 0.0162748f, 0.124789f, 0.0101917f, -0.0547861f, 0.0414249f, -0.0484098f, -0.0963767f, 0.0484124f, -0.00538394f, 0.00789277f, -0.0102249f, 0.104348f, 0.00192805f, 0.00647828f, -0.0461272f, -0.00422982f, 0.0325315f, 0.0211869f, 0.0120108f, 0.0362735f, -0.100353f, -0.106846f, 0.0453949f, 0.108593f, -0.00433587f, 0.0477131f, -0.011734f, -0.00221842f, -0.0186096f, 0.0176472f, 0.0535756f, -0.00753715f, -0.00288954f, 0.0351324f, -0.000401414f, 0.0260439f, 0.0353749f, -0.0214858f, 0.000924544f, 0.000524206f, 0.0411247f, -0.0106592f, 0.0429043f, 0.0430829f, 0.029413f, -0.00455873f, -0.0157798f, 0.0105745f, -0.10904f, -0.0209529f, -0.0605732f, 0.0151213f, 0.0293344f, -0.030329f, 0.0388919f, 0.0830521f, -0.0636824f, 0.0834155f, -0.0213396f, -0.0029129f, 0.0130443f, 0.0276463f, 0.0168069f, 0.0131674f, 0.00569299f, 0.0319241f, -0.215148f, -0.080939f, 0.033579f, -0.0317194f, 0.0329596f, -0.000564258f, 0.0486169f, -0.0763688f, -0.0114993f, -0.0945774f, -0.0518434f, 0.0208386f, -0.059539f, -0.109562f, 0.0544319f, -0.0264226f, -0.090276f, -0.0850728f, -0.0434345f, -0.00627017f, 0.0531473f, 0.214103f, -0.0206121f, 0.0996398f, 0.155764f, -0.00199676f, -0.0115997f, -0.0355156f, 0.113538f, 0.0365645f, 0.0733962f,
        -0.00286558f, -0.000197294f, -0.00342685f, 0.0153883f, -0.0289065f, -0.00108885f, 0.0387835f, 0.00578341f, -0.0262422f, 0.0582573f, -0.0156079f, 0.00150583f, -0.0362458f, 0.0104373f, 0.0113533f, -0.0386981f, -0.00206408f, 0.00844815f, -0.053396f, -0.00516133f, -0.0270117f, -0.27301f, 0.0846544f, -0.00616813f, 0.0871727f, 0.00543487f, 0.00184716f, 0.000983837f, 0.0179262f, 0.0818094f, 0.00281717f, 0.000740774f, -0.19657f, -0.00320105f, -0.00607788f, 0.0319331f, -0.0235907f, -0.0270345f, -0.00849474f, -0.0110374f, -0.0746362f, -0.00860617f, -0.0237307f, -0.00439848f, 0.0264778f, -0.00958103f, 0.019552f, -0.00263648f, 0.0296515f, -0.00113696f, -0.124069f, -0.0114847f, 0.00308178f, 0.0198766f, 0.0900992f, 0.00727182f, -0.0940811f, 0.0216224f, -0.0459814f, 0.017883f, -0.0572063f, 0.0627439f, -0.00563362f, -0.0648773f, -0.00153448f, -0.0913932f, 0.0230685f, 0.00374598f, 0.034431f, 0.112274f, -0.326763f, 0.0153011f, 0.00631464f, 0.00570424f, -0.0595568f, -0.0114755f, -0.0054627f, -0.000223818f, -0.00287856f, -0.0252934f, -0.0108832f, 0.0186247f, -0.0140906f, -0.0239924f, -0.0332168f, -0.00818134f, -0.0261136f, 0.0112761f, -0.0570478f, -0.0484226f, -0.00280576f, -0.140804f, -0.0192205f, 0.0193394f, 0.0043392f, -0.0096851f, -0.0238295f, 0.0496011f, -0.00870349f, 0.0271804f, 0.00735628f, 0.00931979f, -0.000362012f, -0.00038859f, 0.098518f, -0.0510564f, -0.00233872f, 0.00517725f, 0.0101231f, -0.0331861f, 0.0441328f, -0.00924161f, -0.0059294f, -0.0159056f, -0.0810096f, 0.203707f, 0.00935022f, 0.00920423f, 0.0427866f, 0.0270535f, -0.0613705f, 0.0281747f, -0.0292151f, -0.011845f, -0.560809f, -0.0430764f, 0.0249193f, 0.001065f, 0.0495798f, -0.00604974f, -0.0115863f, -0.0841969f, -0.0400231f, -0.0234006f, -0.099013f, 0.0434646f, 0.000907694f, 0.0125445f, 0.0118042f, -0.00467421f, 0.00886041f, 0.0296945f, 0.0324396f, -0.0114072f, 0.0988003f, 0.00847453f, 0.0464346f, -0.00464305f, -0.0289332f, 0.0590643f, -0.0350208f, -0.0899201f, -0.0159029f, -0.0648027f, 0.00696909f, -0.00101221f, 0.0140877f, -0.0855302f, -0.000846032f, -0.0256277f, 0.00854884f, -0.00292961f, 0.209544f, 0.0872828f, 0.0246488f, 0.0291603f, -0.0784974f, 0.00920441f, -0.011242f, -0.0297102f, 0.0152799f, -0.0428288f, -0.0651387f, 0.0138869f, 0.0139815f, 0.0836656f, 0.0361113f, -0.0635471f, -0.0160178f, -0.0220017f, 0.234027f, -0.0400384f, 0.186927f, -0.0295061f, 0.00130944f, -0.0287178f, -0.0214042f, -0.0285818f, 0.0222618f, -0.00368823f, 0.0601194f, -0.0188088f, -0.0146725f, 0.0157483f, 0.21603f, 0.056817f, -0.20685f, -0.0254415f, 0.00525571f, 0.00219887f, 0.0530388f, 0.0607272f, 0.0061378f, -0.113869f, -0.16334f, -0.0464161f, -0.00694523f, -0.00488537f, 0.0286918f, 0.00290496f, 0.178755f, 0.0109929f, 0.110835f, -0.0642967f, -0.0333608f, 0.00169389f, -0.00546941f, 0.00973807f, -0.00576067f, -0.0205257f, 0.0511577f, -0.0266243f, 0.109812f, 0.0471989f, 0.0996845f, 0.0135877f, -0.0794984f, 0.0649346f, 0.0303168f, -0.0011697f, 0.00521801f, 0.0626395f, -0.00297682f, 0.0266726f, -0.000223535f, 0.0116355f, -0.0108245f, 0.000611158f, 0.00728507f, 0.0239288f, -0.00188282f, 0.0150957f, -0.040548f, -0.0589448f, 0.0328252f, -0.0915972f, 0.0805046f, -0.00811939f, 0.0772469f, -0.0716012f, 0.000604462f, 0.047583f, 0.0334997f, -0.000381467f, -0.00726828f, 0.00027943f, -0.0427843f, -0.0568598f, 0.0147649f, -0.00348073f, 0.00288838f, 0.00979242f, -0.00538436f, -0.024106f, 0.00541673f, 0.00529046f, -0.00278852f, -0.0222607f, -0.00626747f, 0.0973789f, -0.0795939f, 0.105127f, -0.337742f, 0.0172115f, 0.00255328f, -0.0330435f, 0.0063678f, 0.0471297f, -0.050865f, -0.00217128f, 0.0139913f, -0.00278459f, 0.0452206f, -0.0122722f, 0.00537665f, 0.0068003f, -0.0241691f, -0.00537261f, 0.00198657f, 0.0288662f, -0.0673232f, -0.00391073f, 0.0160158f, -0.0148616f, 0.00889894f, 0.0278599f, -0.0259723f, -0.0464762f, -0.0699778f, 0.0855682f, -0.00447207f, -0.105144f, -0.000995281f, -0.0146742f, -0.49647f, 0.0685417f, -0.000740646f, 0.0278313f, -0.00761982f, 0.0475931f, -0.0645097f, 0.119236f, -0.0570179f, 0.00915969f, 0.0156965f, 0.101129f, -0.0274397f, 0.0317f, 0.435965f, 0.0895423f, 0.0228896f, 0.0537683f, -0.0312062f, -0.0316729f, 0.00405423f, -0.00417011f, 0.053186f, 0.0124111f, -0.0636419f, -0.059223f, 0.00212677f, -0.00180764f, -0.0184438f, -0.00539991f, -0.0216965f, -0.0297828f, -0.00665945f, 0.0659594f, 0.109878f, -0.0859683f, -0.0195527f, 0.0856906f, 0.113261f, 0.0901811f, 0.00573377f, 0.0357797f, -0.0261576f, 0.0127095f, 0.00452054f, 0.0160191f, 0.0674667f, -0.0187489f, 0.00896214f, -0.00895184f, 0.388793f, 0.0155203f, -0.206128f, -0.0134212f, 0.0159576f, 0.240592f, -0.0244503f, 0.0595618f, 0.0056212f, -0.0505254f, 0.160077f, 0.0021605f, 0.111341f, -0.664956f, 0.031356f, -0.00658282f, -0.431486f, -0.0241319f, -0.437714f, 0.0186697f, 0.0143805f, -0.0139802f, -0.00777148f, 0.0223012f, -0.0458929f, 0.0103136f, 0.0203269f, -0.0121667f, -0.00358236f, -0.0347832f, 0.0310102f, 0.0940264f, 0.0402878f, 0.0779475f, 0.085935f, 0.0506573f, 0.0125433f, 0.00945608f, 0.00711064f, -0.0157027f, -0.00267093f, -0.0460969f, 0.00133153f, 0.0510218f, 0.0568231f, 0.00654478f, -0.0148599f, -0.00556127f, 0.0984337f, 0.0012008f, 0.0401073f, -0.00218267f, -0.0913605f, 0.0250143f, 0.0269926f, -0.00189873f, 0.145338f, -0.0106285f, 0.128684f, 0.0182833f, -0.0104387f, 0.058272f, 0.054818f, -0.0204594f, 0.0514151f, -0.0114196f, 0.0121938f, -0.0135972f, 0.00423344f, 0.0268584f, -0.0233103f, 0.0149913f, 0.00556167f, 0.175006f, 0.0460865f, -0.0531133f, -0.00530817f, 0.00775018f, -0.00568381f, 0.00309299f, 0.00404426f, 0.0611169f, 0.04162f, 0.0620172f, 0.0113454f, 0.0556293f, -0.000326539f, -0.0136839f, -0.00373327f, 0.0962103f, -0.0169842f, 0.0247842f, 0.0442757f, 0.0244144f, -0.0176649f, -0.00554654f, -0.0050203f, -0.0177601f, -0.02368f, 0.0243078f, -0.0571087f, 0.0184628f, -0.0841841f, 0.0331607f, 0.0279732f, -0.0822138f, 0.0293232f, -0.0722001f, 0.0163439f, 0.0191851f, 0.414194f, 0.456304f, 0.097353f, 0.033467f, -0.010367f, -0.00362604f, -0.00940526f, 0.0541993f, -0.0126803f, -0.0284043f, -0.126488f, 0.0276941f, -0.0072592f, -0.0112239f, 0.200614f, -0.0674165f, 0.0152713f, -0.0543701f, -0.0742834f, -0.0453187f, -0.0254072f, -0.0692672f, 0.0332971f, -0.0228297f, -0.000965714f, 0.0732683f, 0.0640799f, 0.00158938f, 0.047803f, -0.00266977f, -0.0100275f, -0.00643167f, -0.0383495f, -0.00409583f, 0.0385844f, 0.0659188f,
        0.0063133f, -0.00408226f, 0.121465f, 0.0301708f, -0.0181853f, 0.0601681f, 0.00325393f, 0.10642f, -0.0275263f, -0.0194839f, -0.0252979f, 0.0217105f, 0.0386137f, 0.0112424f, 0.0430641f, 0.0730034f, 0.0354242f, 0.013652f, -0.0293887f, 0.142649f, -0.0690173f, -0.0961422f, 0.0442838f, 0.0452969f, 0.118274f, 0.0323701f, 0.0187156f, 0.5255f, 0.0118736f, 0.225357f, -0.0130602f, -0.0104742f, -0.07411f, -0.114514f, -0.0436895f, 0.00986579f, -0.0838205f, -0.101698f, -0.00483559f, -0.00391671f, -0.0699783f, -0.0195803f, 0.0459022f, -0.0091508f, 0.0073998f, -0.0577818f, 0.0674949f, 0.0137614f, 0.0715333f, 0.00271481f, -0.00891188f, -0.0212177f, 0.0437716f, 0.0257086f, 0.0345469f, -0.180349f, -0.0603965f, -0.147289f, -0.00330522f, 0.0067096f, -0.0179399f, 0.0182082f, -0.0270762f, 0.0402878f, -0.0166916f, -0.0948335f, 0.029574f, 0.0969981f, 0.0529901f, 0.00293059f, -0.154666f, 0.0407095f, 0.0316545f, -0.0062415f, -0.0351574f, -0.0147547f, -0.0135113f, 0.00357694f, 0.0517612f, -0.101499f, -0.00291564f, -0.0056001f, -0.00857672f, -0.0101505f, -0.0323477f, -0.0263152f, -0.0116552f, 0.0247082f, 0.0227123f, -0.10951f, -0.0328793f, 0.411161f, -0.0130315f, -0.0227835f, 0.0106074f, -0.00307627f, 0.00495261f, 0.0545998f, 0.000595861f, -0.0242671f, 0.0299187f, 0.00166324f, -0.00666328f, -0.0078437f, 0.0280452f, -0.16448f, -0.0143541f, 0.026909f, -0.193269f, -0.0355148f, 0.0118665f, -0.0365043f, -0.00810059f, -0.0352678f, -0.0630561f, 0.0280126f, 0.30164f, 0.0875995f, 0.0694396f, 0.0103573f, -0.0283321f, -0.621525f, -0.0445668f, -0.0148087f, -0.313831f, -0.00408616f, 0.0349075f, 0.0231337f, 0.142115f, 0.00382164f, 0.0393434f, -0.108881f, -0.0101964f, -0.0303501f, -0.106503f, 0.0308691f, -0.0197364f, 0.0091609f, 0.00739707f, -0.021932f, 0.00100097f, 0.00910001f, -0.0272304f, 0.0244325f, -0.0534487f, -0.0124806f, 0.102616f, -0.0300018f, -0.0371498f, -0.0484335f, -0.0434477f, -0.0806446f, -0.0323094f, 0.0210301f, 0.016248f, 0.0884761f, 0.0521384f, -0.306267f, -0.0181587f, 0.0638134f, 0.00266205f, 0.0659853f, 0.0215718f, 0.030898f, -0.010891f, 0.0265176f, -0.0440084f, 0.0334551f, -0.0404191f, -0.05042f, 0.0401076f, 0.00569889f, 0.0642698f, 0.0118167f, -0.152626f, -0.0383063f, -0.241934f, -0.14967f, 0.000835922f, -0.0176463f, 0.00669299f, -0.100216f, 0.0636827f, -0.0246564f, 0.0233452f, 0.00916313f, -0.0360494f, -0.0143271f, 0.00748104f, 0.00808922f, 0.120031f, -0.0139543f, -0.0895863f, -0.0414794f, 0.143243f, -0.0137803f, 0.0207675f, -0.0347851f, 0.0721874f, -0.0414808f, -0.116213f, 0.00107106f, 0.0103554f, -0.13586f, -0.290486f, 0.00166402f, -0.015201f, -0.00145561f, -0.0154914f, 0.00163743f, 0.0822632f, 0.08017f, 0.0710966f, -0.013158f, -0.0632138f, -0.0111834f, -0.0178201f, 0.0112061f, -0.00430423f, -0.0674515f, 0.214633f, -0.00585192f, -0.0351569f, 0.375032f, 0.0448701f, 0.0256456f, 0.0743934f, 0.0211866f, -0.00896532f, -0.0415844f, 0.0122347f, 0.0118991f, -0.0877453f, 0.0304085f, -0.00665392f, -0.00567859f, -0.00832385f, 0.00138205f, 0.0402719f, -0.00329125f, -0.0122391f, 0.0130672f, -0.0699987f, -0.0336706f, 0.0130345f, -0.256598f, -0.00998923f, -0.0732391f, 0.16722f, -0.0470782f, 0.016357f, 0.0118742f, -0.0706653f, 0.00409f, -0.0124226f, 0.000505835f, -0.0507414f, 0.00258108f, 0.0198879f, 0.000320695f, 0.0112645f, 0.00723067f, -0.0107117f, -0.00964231f, 0.014985f, -0.000720747f, -0.00563631f, -0.128197f, -0.00191921f, 0.100766f, -0.0177464f, 0.0910596f, 0.132686f, 0.0851709f, 0.0140803f, -0.0459295f, 0.00891749f, 0.0917738f, -0.0520881f, -0.00429575f, -0.0104893f, -0.0285219f, 0.0370703f, -0.0241567f, 0.0214466f, 0.0260263f, 0.112436f, -0.0221967f, 0.003362f, 0.00552892f, -0.0382231f, 0.00763609f, 0.0270099f, -0.028698f, -0.00121651f, 0.000527033f, -0.0406943f, -0.0840261f, -0.00983556f, -0.0288269f, 0.00269151f, -0.136611f, 0.0220631f, -0.00476321f, 0.0281217f, 0.0243983f, -0.00436437f, 0.00491977f, 0.0540143f, 0.0410553f, -0.00945594f, -0.0711867f, -0.011407f, -0.0290617f, 0.0077444f, -0.0194761f, -0.0353022f, 0.0242323f, 0.121606f, 0.136937f, 0.117977f, 0.0648052f, 0.000369128f, -0.0286182f, -0.000851573f, -0.0675435f, 0.0374786f, 0.0108061f, -0.00134871f, -0.0419874f, 0.0271549f, -0.21822f, 0.268321f, -0.00535237f, 0.011111f, -0.0614932f, 0.0500974f, 0.0900748f, 0.0334851f, -0.101783f, -0.00498551f, -0.0075128f, 0.00031712f, 0.0485839f, 0.000919265f, 0.0326066f, -0.023036f, 0.0096988f, 0.0178391f, 0.0861196f, 0.0466213f, -0.0299909f, -0.0991148f, -0.0230341f, 0.334094f, -0.0382573f, 0.0395579f, -0.00590484f, 0.0206429f, 0.246985f, -0.0283786f, 0.0598143f, -0.0353774f, 0.091151f, 0.0944889f, 0.00249664f, 0.202462f, -0.00569812f, 0.00865333f, -0.00812537f, -0.188173f, -0.0627191f, -0.28001f, 0.00917071f, 0.0506412f, 0.0010405f, 0.0678395f, 0.16542f, -0.00219039f, 0.0110519f, -0.00379539f, 0.00535911f, -0.00791708f, -0.000717427f, -0.0325235f, 0.0842137f, -0.020968f, 0.192455f, 0.0856024f, 0.132173f, -0.00232728f, 0.0647325f, 0.104932f, -0.0235684f, 0.00335134f, 0.00515333f, 0.192284f, 0.0592319f, 0.143246f, -0.00214825f, -0.168829f, -0.0149753f, 0.00881463f, 0.00489184f, 0.0030815f, -0.0645487f, -0.236596f, 0.0211161f, 0.428909f, -0.0184283f, 0.150971f, -0.00403509f, 0.0892136f, 0.0527521f, -0.00892411f, 0.257531f, 0.0159127f, -0.0153799f, 0.0299046f, 0.00748111f, 0.02268f, -0.0283898f, -0.0224564f, -0.00329609f, -0.0642335f, 0.0385503f, 0.00387719f, -0.0795388f, 0.0385978f, 0.0338672f, -0.00181007f, 0.500546f, 0.0174027f, -0.00941603f, 0.00119533f, 0.161396f, 0.0277067f, -0.0113644f, 0.00243689f, 0.0240222f, 0.00074696f, -0.00329644f, 0.00571551f, 0.353842f, -0.0345694f, 0.0954816f, 0.022245f, 0.0639779f, -0.0209006f, -0.0100804f, -0.0223871f, 0.00248849f, -0.0231191f, -0.105286f, -0.0150994f, 0.00230265f, -0.0295301f, 0.0119341f, 0.00911531f, 0.0540066f, 0.0076047f, -0.0945892f, 0.0196067f, -0.0357786f, 0.0719775f, -0.0972845f, 0.142406f, -0.18177f, 0.00491428f, 0.000342362f, -0.0186926f, 0.0489506f, -0.0333847f, -0.017827f, -0.00585373f, 0.0250148f, -0.0496847f, 0.00595432f, 0.180951f, -0.0459607f, -0.0360709f, -0.168328f, -0.0724864f, -0.161582f, 0.0156965f, -0.0463856f, 0.00603378f, -0.0396591f, 0.100121f, 0.00849666f, 0.0438226f, 0.0247446f, 0.0309354f, -0.0876779f, -0.0223912f, 0.0149475f, -0.0619022f, -0.0198987f, 0.0258675f, 0.0760512f,
        0.0237833f, 0.00298876f, 0.0487694f, 0.00950606f, -0.074622f, 0.0192038f, -0.0202395f, 0.105125f, -0.0154085f, 0.0355691f, 0.00281225f, 0.00531638f, 0.0101454f, 0.0510713f, 0.0313131f, -3.24692e-05f, 0.0563302f, -0.00384794f, -0.0967057f, -0.00911184f, -0.034748f, -0.00885298f, -0.00145702f, 0.00841001f, -0.00386897f, 0.00954715f, 0.0060942f, -0.00779779f, 0.0341911f, 0.0373562f, 0.000677265f, -0.0620633f, 0.00208294f, -0.0215586f, -0.085074f, 0.0143441f, -0.0186877f, 0.00127867f, -0.01249f, -0.00504883f, -0.00104019f, 0.0121985f, 0.000512828f, -0.00772995f, 0.00468516f, -0.0139477f, -0.0211804f, 0.210879f, 0.00785329f, -0.000516933f, -0.00212956f, -0.0162727f, 0.00414868f, 0.0109553f, 0.000250999f, -0.00637749f, -0.00108913f, -0.00648906f, -0.0123977f, 0.0104616f, 0.0241319f, 0.0770632f, 0.00195405f, -0.00752428f, -0.0405081f, -0.0883033f, 0.0394711f, 0.0062544f, 0.0315002f, -0.0138193f, -0.0353362f, 0.00803457f, 0.0055575f, -0.00122304f, -0.00591179f, -0.000313378f, -0.00928775f, 0.00167335f, 0.00110711f, 0.0102733f, -0.0102128f, -0.0332447f, -0.0050578f, -0.0365285f, 0.00129188f, -0.00545454f, -0.0488076f, -0.0522689f, -0.0028496f, 0.0269232f, -0.00264586f, 0.00549725f, 0.0937312f, -0.0097157f, 0.000703438f, -0.0316939f, 0.00265145f, 0.00747435f, 0.00703635f, -0.0498706f, 0.0260258f, 0.00486406f, 0.00831138f, 0.00331964f, -0.0116462f, -0.000328743f, -0.0193854f, 0.012874f, -0.0140591f, 0.00294906f, 0.167637f, -0.00563081f, 0.00047881f, -0.0132155f, -0.088562f, -0.00763682f, 0.00861545f, 0.0484862f, 0.118604f, 0.00888342f, -0.0480975f, -0.0108402f, -0.00768345f, -0.214419f, -0.045855f, 0.000607434f, 0.00143275f, 0.000233664f, 0.00111974f, 0.0283561f, -0.0137152f, 0.035663f, -0.0231469f, 0.0205628f, 0.0685008f, 0.0106492f, 0.00590557f, -0.00685771f, 0.00424108f, 0.000113577f, 0.00595773f, 0.00665598f, 0.000441705f, -0.00402036f, -0.0262544f, 0.00611645f, 0.0116063f, -0.00424871f, 0.0342696f, 0.0381022f, -0.0588067f, -9.04306e-05f, 0.013434f, 0.0049054f, 0.0123942f, -0.000403249f, 0.0504587f, -0.00181204f, 0.00841684f, 0.0187689f, 0.0174106f, 0.00611652f, 0.00976013f, 0.000955711f, 0.00209072f, -0.0257193f, -0.0127599f, 0.00699173f, -0.0153516f, -0.00193625f, 0.0528177f, 0.0170662f, 0.0746572f, 0.00809554f, -0.027025f, -0.0257472f, -0.00256271f, -0.0890082f, -0.00221022f, -0.00891542f, -0.00903598f, -0.0144857f, 0.0554675f, -0.00986486f, 0.00189685f, 5.93501e-05f, 0.00462237f, 0.00532594f, 0.00433364f, -0.003124f, 0.04f, -0.000328486f, -0.0648411f, -0.00377033f, 0.139774f, 0.00230164f, 0.0115385f, 0.0125043f, 0.148022f, -0.0284796f, -0.00155402f, -0.00387695f, 0.00829478f, -0.0471497f, -0.0015643f, -0.00582674f, -0.00431319f, 0.000878919f, 0.00687072f, -0.00301133f, 0.00398096f, -0.00563914f, -0.0026393f, -0.00377055f, -0.0609272f, -0.118688f, 0.00517703f, 0.0836725f, -0.012182f, -0.0512972f, 0.0119928f, 0.0247734f, -0.0427426f, 0.0341825f, 0.0698612f, 0.00279914f, -0.00847926f, -0.0226391f, 0.020679f, -0.00144619f, -0.0104832f, 0.0195441f, 0.000150691f, 0.0815801f, -0.00616593f, 0.00379428f, -0.00447982f, 0.00261409f, 0.0600844f, -0.0213836f, -0.00804557f, 0.00325642f, 0.00854879f, -0.0814344f, -0.027769f, -0.00191851f, 0.00536533f, -0.0164033f, -0.00257131f, -0.00205376f, -0.0200541f, -0.0128954f, -0.00532982f, 0.0022407f, -0.00130887f, 0.00425618f, -0.00845818f, -0.00126148f, -0.0107566f, 0.00104842f, -0.00435674f, 0.00433842f, -0.0109865f, 0.000301519f, 0.00589863f, -0.00851759f, -0.00137109f, -0.0256632f, 0.0120122f, -0.00451766f, -0.0132172f, 0.0204377f, 0.00862719f, -0.00529603f, 0.0007616f, -0.00779072f, 0.000307369f, 0.0161384f, 0.0140168f, -0.00223271f, -0.0234216f, 0.00152691f, 0.00407567f, -0.00575267f, -0.0169706f, 0.00373715f, -0.0130443f, 0.0149063f, -0.00592504f, -0.00101738f, -0.00432452f, 0.00608682f, -0.00623923f, -0.0048846f, 0.00141049f, -0.00787022f, -0.00325903f, -0.00925192f, 4.10188e-05f, -0.00650579f, -0.00344007f, -0.00507379f, -0.010943f, 0.0033921f, 0.0262149f, -0.0109309f, -0.00218072f, 0.00487267f, -0.00424018f, 0.0190863f, -0.0205672f, -0.00521787f, -0.749656f, 0.0045255f, -0.0111087f, -0.00594957f, -0.00784532f, -0.00218566f, -0.00261733f, 0.00115839f, 0.00810127f, -0.00685174f, -0.000515265f, 0.00996413f, 0.00908507f, -0.010911f, 0.0199673f, 0.00424915f, -0.0168506f, -0.0127626f, -0.0068238f, 0.0141051f, -0.0106615f, 0.00332799f, 0.00636155f, -0.0260333f, 0.00595097f, 0.0191085f, -0.0049198f, 0.00793315f, -0.00309666f, 0.0137166f, -0.00473366f, 0.0127659f, 0.000838826f, 0.0352708f, -0.00566433f, 0.00439918f, 0.00403144f, -0.0103773f, 0.000578005f, -0.00181792f, -0.0300049f, -0.00661571f, 0.0085107f, 0.00894339f, 0.00861617f, 0.00351911f, 0.016009f, -0.00165849f, 0.00140448f, 0.00854556f, -0.000467159f, 0.00526625f, 0.0113457f, -0.000892589f, -0.00943319f, 0.016298f, 0.0129145f, 0.00977724f, -0.00864554f, -0.0149309f, 0.0109739f, 0.00925517f, 0.00301191f, -0.00253138f, -0.0198261f, 0.00383641f, 0.00511284f, -0.0561408f, -0.0281949f, -0.00444545f, -0.00338158f, -0.00161292f, -0.00978353f, 0.00446439f, 0.000485823f, 0.000591379f, 0.00729576f, -0.024535f, 0.00937071f, 0.00193014f, 0.00812366f, -0.015649f, -0.00101637f, 0.0112705f, 0.00182169f, -0.00906464f, 0.0080621f, -0.0130414f, -0.000293886f, -0.00548405f, -0.00557287f, -0.00444211f, 0.000131822f, -0.0116247f, 0.00918694f, 0.00706824f, -0.00459982f, -0.00134241f, 0.00769962f, -0.000905408f, -0.00643464f, 0.00195699f, 0.0103661f, 0.0117231f, 0.00141366f, 0.013737f, -0.00475491f, -0.00389627f, -0.008428f, -0.00336822f, -0.0123985f, -0.00384732f, -0.00772105f, -0.00399041f, 0.00441658f, -0.0179348f, 0.00088589f, 0.00130237f, -0.00910743f, -0.000932973f, -0.000705488f, -0.00845157f, -0.00409019f, -0.00198943f, -0.00037801f, -0.0110968f, -0.00639611f, 0.00967489f, -0.00286205f, -0.00142743f, 0.00952024f, 0.0067011f, -0.00771389f, 0.000101275f, 0.00173372f, 0.000959312f, 0.00841471f, 0.00336334f, 0.00371336f, 0.00482025f, -0.00711383f, 0.00583148f, 0.0108545f, -0.000470039f, -0.0110626f, 0.00324574f, 0.025979f, 0.0153801f, -0.00239289f, -0.0364105f, -0.0252222f, 0.00766028f, -0.000371992f, -0.00263989f, 0.0215774f, 0.0230998f, -0.00223724f, -0.000281751f, -0.00482297f, -0.0175295f, -0.00712851f, 0.0106509f, 0.00430235f, 0.00410187f, 0.00823292f, 0.00280169f, 8.28998e-05f, -0.00169138f, -0.00976853f, -0.00530213f, -0.00814388f, 0.0013187f, 0.00816157f, 0.00138731f, -2.68979e-05f, -0.0103893f, -0.0500543f, 0.000847671f, 0.00327953f, 0.00418289f, 0.0180997f, -0.00027566f, -0.00544788f, -0.0076323f, -0.00551657f, -0.00599236f, -0.0127374f, -0.0174632f,
    };

    std::vector<float> input_data2 = {
        0.226067f,
        -0.213117f,
        -0.228827f,
        0.0765218f,
        -0.0891241f,
        0.344041f,
        0.0207557f,
        -0.00652239f,
        -0.102098f,
        -0.182902f,
        0.196429f,
        0.135405f,
        0.0946788f,
        -0.524967f,
        0.274247f,
        0.195996f,
        0.218358f,
        -0.264805f,
        -0.172852f,
        -0.259562f,
        0.0541039f,
        -0.00698586f,
        -0.10335f,
        -0.0509606f,
        -0.0952293f,
        0.307304f,
        0.0449068f,
        -0.103906f,
        0.0788517f,
        -0.197936f,
        -0.11548f,
        -0.331254f,
        -0.231074f,
        -0.0954558f,
        -0.0851322f,
        -0.349265f,
        -0.0737912f,
        0.127517f,
        -0.384151f,
        0.045742f,
        -0.0797452f,
        -0.408111f,
        -0.301897f,
        -0.150026f,
        -0.0893552f,
        -0.194268f,
        -0.148536f,
        0.627003f,
        -0.181857f,
        -0.378125f,
        0.0897165f,
        -0.12586f,
        0.0681846f,
        -0.619639f,
        -0.126204f,
        -0.104365f,
        0.332828f,
        -0.139087f,
        0.0427636f,
        -0.0607021f,
        0.0202485f,
        0.0134034f,
        0.0509213f,
        -0.0549845f,
        -0.232839f,
        0.107752f,
        -0.102923f,
        0.529762f,
        -0.00969623f,
        -0.230437f,
        0.0347148f,
        0.183061f,
        0.161027f,
        -0.00635875f,
        -0.0919441f,
        -0.527769f,
        -0.280842f,
        0.10766f,
        -0.0720151f,
        -0.162666f,
        0.0319142f,
        0.240846f,
        0.0692376f,
        0.470951f,
        0.184244f,
        -0.57936f,
        -0.212887f,
        -0.528276f,
        -0.00906647f,
        0.273247f,
        0.212606f,
        0.217403f,
        0.141283f,
        0.106187f,
        0.34241f,
        -0.106528f,
        -0.0292508f,
        -0.133695f,
        0.375936f,
        -0.0659701f,
        0.211015f,
        0.0380058f,
        -0.153224f,
        0.0285997f,
        -0.211682f,
        -0.0773371f,
        0.321415f,
        0.649155f,
        0.0767063f,
        -0.744861f,
        -0.161323f,
        -0.244903f,
        0.0927423f,
        0.31863f,
        0.941846f,
        0.0722605f,
        0.0945846f,
        -0.127138f,
        0.261468f,
        -0.171585f,
        -0.241784f,
        0.0771862f,
        0.197024f,
        0.161495f,
        0.430723f,
        0.0580771f,
        0.207676f,
        -0.159349f,
        -0.0989325f,
        0.856337f,
        -0.0617799f,
        -0.0957737f,
        0.145259f,
        0.238383f,
        0.2104f,
        0.43008f,
        -0.0680796f,
        0.369157f,
        -0.0556704f,
        0.0373768f,
        0.0570986f,
        0.14268f,
        -0.0244769f,
        -0.0465839f,
        0.115875f,
        0.0281787f,
        0.303995f,
        -0.0961961f,
        0.194476f,
        0.386232f,
        -0.00797209f,
        0.340192f,
        -0.125736f,
        0.239907f,
        0.112839f,
        -0.333855f,
        0.117835f,
        0.821566f,
        0.0994434f,
        0.194981f,
        -0.0887003f,
        -0.0775066f,
        0.0356492f,
        0.181386f,
        -0.309338f,
        -0.0755519f,
        -0.053338f,
        -0.104813f,
        0.00407328f,
        0.318923f,
        -0.0287042f,
        0.0418845f,
        0.0237666f,
        0.162007f,
        -0.241018f,
        -0.0431646f,
        0.277778f,
        0.224636f,
        -0.187592f,
        0.119577f,
        -0.056689f,
        -0.0592898f,
        0.148013f,
        0.182265f,
        -0.0118085f,
        -0.113206f,
        0.220402f,
        0.134212f,
        -0.128761f,
        -0.0865038f,
        -0.412019f,
        -0.045835f,
        -0.102225f,
        -0.497989f,
        0.209973f,
        0.117021f,
        -0.24728f,
        -0.121575f,
        -0.18151f,
        0.138296f,
        0.0448733f,
        0.0895001f,
        -0.0787051f,
        0.00202249f,
        0.368028f,
        0.118236f,
        0.218559f,
        -0.030864f,
        0.590104f,
        0.0625101f,
        0.0510301f,
        -0.115514f,
        -0.149621f,
        0.0977415f,
        -0.443171f,
        -0.102979f,
        0.278304f,
        -0.0942679f,
        -0.170019f,
        -0.0328404f,
        -0.0712928f,
        0.063828f,
        0.0632373f,
        0.319856f,
        -0.374302f,
        0.199073f,
        -0.16688f,
        0.159463f,
        -0.135776f,
        -0.272128f,
        -0.0254539f,
        0.00786319f,
        -0.225175f,
        0.195573f,
        0.434743f,
        0.0534771f,
        -0.484686f,
        0.673282f,
        -0.168741f,
        0.0898199f,
        -0.0216728f,
        -0.0784748f,
        0.187535f,
        -0.435589f,
        0.390775f,
        -0.0944115f,
        -0.0742034f,
        -0.0246503f,
        -0.0557492f,
        -0.100895f,
        0.823227f,
        0.0371927f,
        -0.364289f,
        0.460798f,
        -0.017548f,
        0.172304f,
        0.532184f,
        0.0199246f,
        -0.0948896f,
        -0.136876f,
        -0.0614491f,
        0.307103f,
        2.44539f,
        -0.186546f,
        -0.237352f,
        0.2261f,
        -0.0247159f,
        -0.42788f,
        -0.0296576f,
        -0.20106f,
        0.103752f,
        -0.196205f,
        0.256877f,
        -0.0783613f,
        0.198236f,
        -0.170475f,
        0.00141333f,
        -0.665422f,
        -0.293426f,
        -0.167652f,
        -0.0532202f,
        -0.111295f,
        -0.267167f,
        -0.238999f,
        -0.0177677f,
        0.0850333f,
        -0.068634f,
        -0.0528774f,
        0.220314f,
        0.0237442f,
        0.318326f,
        0.222658f,
        0.199311f,
        -0.00248508f,
        -0.00227757f,
        -0.203799f,
        -0.652534f,
        -0.287742f,
        -0.0809664f,
        0.10783f,
        -0.0225144f,
        -0.00212286f,
        0.134815f,
        0.0838675f,
        0.12512f,
        -0.00969528f,
        0.0849158f,
        0.230933f,
        -0.126511f,
        -0.482854f,
        0.33909f,
        0.197446f,
        -1.01119f,
        -0.271206f,
        0.749912f,
        -0.38239f,
        0.252609f,
        -0.0157146f,
        -0.0394711f,
        0.218464f,
        0.142877f,
        0.0720711f,
        -0.117446f,
        0.106793f,
        0.356202f,
        -0.048713f,
        0.405457f,
        -0.0116166f,
        -0.32676f,
        -0.0254017f,
        -0.0573934f,
        0.0650745f,
        -0.307225f,
        0.0331111f,
        -0.229207f,
        0.397605f,
        -0.0752643f,
        0.281572f,
        0.0555904f,
        0.0287739f,
        -0.109718f,
        0.184021f,
        -0.114731f,
        -0.290246f,
        0.453322f,
        -0.70934f,
        -0.0324739f,
        0.105673f,
        -0.60454f,
        0.277497f,
        -0.0987777f,
        0.0701585f,
        -0.263903f,
        0.115963f,
        0.0970603f,
        0.404111f,
        0.245729f,
        0.161908f,
        0.153264f,
        -0.218512f,
        -0.028275f,
        0.102634f,
        0.320952f,
        0.184322f,
        0.0650539f,
        -0.0584871f,
        -0.161583f,
        0.196085f,
        -0.473119f,
        -0.0859971f,
        0.305992f,
        -0.106663f,
        0.102449f,
        -0.0325308f,
        -0.11967f,
        0.15366f,
        -0.233337f,
        0.00871865f,
        -0.166247f,
        -0.647049f,
        0.325162f,
        -0.0751572f,
        0.0708473f,
        0.0753089f,
        -0.0773621f,
        -0.664742f,
        0.291293f,
        0.25449f,
        -0.477115f,
        -0.349269f,
        -0.0686682f,
        -0.0517202f,
        0.226158f,
        -0.530734f,
        -0.395195f,
        -0.282488f,
        -0.126664f,
        -0.304453f,
        -0.338142f,
        -0.0181436f,
        -0.3111f,
        -0.0463679f,
        0.239638f,
        0.361935f,
        0.00467099f,
        0.011375f,
        -0.0595875f,
        -0.283181f,
        0.122049f,
        0.144895f,
        0.0969424f,
        -0.0195244f,
        0.292317f,
        0.0288714f,
        0.0865664f,
        -0.197983f,
        -0.248603f,
        0.0967477f,
        -0.169749f,
        0.226406f,
        1.32286f,
        -0.229499f,
        -0.219579f,
        -0.128554f,
        -0.0271513f,
        0.1842f,
        0.11988f,
        -0.47793f,
        0.130466f,
        0.187302f,
        -0.333369f,
        0.033424f,
        -0.0227941f,
        -0.362701f,
        -0.103464f,
        0.16568f,
        0.00879847f,
        -0.317345f,
        0.241489f,
        -0.366614f,
        -0.118065f,
        0.77793f,
        0.164553f,
        0.292792f,
        -0.208979f,
        -0.0236901f,
        0.319615f,
        0.19925f,
        -0.00983736f,
        -0.0232651f,
        0.317631f,
        -0.13003f,
        1.84825f,
        0.130958f,
        0.105821f,
        0.0604396f,
        -0.0542807f,
        -0.0620607f,
        0.0121423f,
        -0.175904f,
        -0.277185f,
        0.0154553f,
        -0.106009f,
        -0.0587396f,
        -0.0595878f,
        -0.0184346f,
        -0.742679f,
        0.594318f,
        0.0509102f,
        -0.143123f,
        0.073118f,
        -0.454019f,
        -0.186053f,
        0.190839f,
        -0.0799592f,
        0.147735f,
        0.460548f,
        0.566367f,
        -0.249098f,
        -0.036917f,
        -0.121617f,
        0.282431f,
        0.125625f,
        -0.0407198f,
        0.41742f,
        0.0344667f,
        0.375503f,
        -0.330054f,
        0.651581f,
        -0.336214f,
        0.0678401f,
        -0.0432715f,
        -0.0668946f,
        -0.063373f,
        -0.100422f,
        -0.465889f,
        -0.070476f,
        -0.330114f,
        -0.686191f,
        -0.491133f,
        -0.281676f,
        0.203388f,
        -0.260101f,
        0.207742f,
        -0.16209f,
        0.0490458f,
        0.513714f,
        -0.118339f,
        -0.497515f,
        -0.354277f,
        0.114791f,
        0.147685f,
        0.486502f,
        -0.294311f,
        0.318928f,
        -0.0873725f,
        -0.347909f,
        0.0376368f,
        -0.330455f,
        -0.181159f,
        0.336051f,
        0.332889f,
        -0.0234334f,
        -0.222394f,
        -0.20525f,
        -0.140286f,
        -0.0351668f,
        -0.15004f,
        -0.0625771f,
        -0.0735099f,
        0.0579021f,
        0.353291f,
        -0.416761f,
        0.00784921f,
        0.0550992f,
        0.0785671f,
        0.0327464f,
        0.174488f,
        -0.643194f,
        -0.00167506f,
        -0.18351f,
        -0.77283f,
        -0.141698f,
        -0.283943f,
        0.0703258f,
        0.0731425f,
        0.0893804f,
        0.45594f,
        -0.119529f,
        -0.13004f,
        0.0286476f,
        -0.143618f,
        0.0157178f,
        -0.254329f,
        0.32324f,
        -0.0795797f,
        0.531658f,
        -0.0318883f,
        -0.113471f,
        0.0562344f,
        0.0250965f,
        -0.31784f,
        0.312777f,
        0.217587f,
        0.00470227f,
        0.0759693f,
        -0.244138f,
        0.593975f,
        -0.115257f,
        -0.31279f,
        0.293078f,
        0.0778797f,
        0.105953f,
        0.000129977f,
        0.00625674f,
        0.14876f,
        -0.266717f,
        -0.248158f,
        -0.139808f,
        -0.151966f,
        0.181941f,
        0.178033f,
        -0.0492492f,
        0.346183f,
        0.13544f,
        -0.125893f,
        0.213388f,
        -0.320162f,
        -0.113014f,
        0.1089f,
        -0.0229706f,
        -0.253305f,
        -0.108912f,
        -0.442676f,
        0.194433f,
        0.061225f,
        0.11757f,
        0.16437f,
        0.1215f,
        -0.457864f,
        0.0721076f,
        -0.149889f,
        -0.0636956f,
        0.00125577f,
        -0.0332369f,
        0.0629046f,
        0.111954f,
        0.195584f,
        0.0675324f,
        -0.709152f,
        -0.200995f,
        -0.0563228f,
        -0.217369f,
        0.528605f,
        0.895751f,
        -0.00650362f,
        0.152248f,
        0.349439f,
        -0.0111444f,
        -0.0732277f,
        -0.0588748f,
        0.0367729f,
        -0.0289346f,
        0.317606f,
        0.391562f,
        -0.208744f,
        0.230341f,
        -0.44191f,
        0.0197931f,
        0.709046f,
        -0.224512f,
        -0.641606f,
        -0.0249767f,
        0.0258922f,
        0.0138068f,
        -0.106175f,
        0.0353294f,
        0.402765f,
        0.0908363f,
        -0.114702f,
        0.0693749f,
        0.236688f,
        -0.149579f,
        -0.290914f,
        0.0249069f,
        -0.0761767f,
        0.566583f,
        -0.0756051f,
        -0.0730483f,
        0.60233f,
        -0.0485716f,
        0.445732f,
        -0.652175f,
        0.176092f,
        0.0424668f,
        -0.464009f,
        0.0419261f,
        0.437008f,
        -0.0949722f,
        0.322407f,
        0.00229612f,
        -0.0698273f,
        0.279531f,
        0.0615138f,
        0.029985f,
        0.0397836f,
        -0.0450066f,
        -0.153192f,
        -0.00434645f,
        0.235137f,
        -0.430068f,
        0.0479958f,
        0.0538638f,
        0.0335953f,
        0.059768f,
        -0.00594587f,
        0.313076f,
        0.173102f,
        -0.285881f,
        0.0860142f,
        -0.0656319f,
        0.0903421f,
        0.285596f,
        -0.00867567f,
        0.140828f,
        0.0679629f,
        -0.0386886f,
        -0.0432885f,
        -0.0523634f,
        -0.202516f,
        -0.393785f,
        0.0862689f,
        0.196867f,
        -1.25594f,
        0.407078f,
        0.0883489f,
        -0.145128f,
        -0.0270446f,
        -0.436027f,
        0.124345f,
        0.0216037f,
        -0.0341296f,
        -0.0550939f,
        -0.0143608f,
        0.269727f,
        -0.316092f,
        0.24142f,
        -0.0602843f,
        0.677077f,
        0.015578f,
        -0.0866818f,
        -0.0114968f,
        0.233375f,
        0.0768293f,
        -0.305731f,
        -0.101196f,
        0.116959f,
        -0.705778f,
        -0.274866f,
        0.0159985f,
        -0.0331097f,
        -0.165272f,
        0.00676643f,
        0.194435f,
        -0.644084f,
        -0.0719713f,
        -0.38506f,
        -0.0766179f,
        -0.232665f,
        -0.184962f,
        -0.0873918f,
        0.123643f,
        -0.10653f,
        -0.0842152f,
        0.173415f,
        0.0549799f,
        -0.246653f,
        -0.753266f,
        -0.17178f,
        -0.315154f,
        -0.284002f,
        -0.0944792f,
        0.513692f,
        -0.555574f,
        0.207956f,
        0.108376f,
        -0.322661f,
        -0.181751f,
        0.0175992f,
        -0.067274f,
        -0.0244339f,
        0.264443f,
        -0.307399f,
        0.435229f,
        -0.0912658f,
        0.41608f,
        0.203329f,
        0.0611729f,
        -0.11855f,
        -0.0901752f,
        0.219847f,
        -0.00417572f,
        2.35094f,
        0.0649107f,
        -0.0693629f,
        0.118256f,
        0.0929001f,
        -0.295607f,
        0.102728f,
        0.0786511f,
        -0.164747f,
        0.0778936f,
        0.269988f,
        -0.0515926f,
        0.147636f,
        -0.121939f,
        -0.0303203f,
        -0.702988f,
        -0.220738f,
        -0.104178f,
        0.102754f,
        -0.138891f,
        -0.202057f,
        -0.130212f,
        -0.0471592f,
        0.0441513f,
        -0.0329241f,
        0.124936f,
        0.0593826f,
        0.209008f,
        0.336101f,
        0.360896f,
        0.118351f,
        -0.169321f,
        -0.00791483f,
        -0.584241f,
        -0.507303f,
        -0.103466f,
        0.031976f,
        0.17687f,
        -0.0595649f,
        0.126475f,
        -0.046226f,
        0.143653f,
        -0.0457821f,
        0.0206962f,
        -0.0371797f,
        0.179695f,
        0.1175f,
        -0.551365f,
        0.187117f,
        -0.385674f,
        -0.983871f,
        -0.42516f,
        0.0679211f,
        -0.199898f,
        0.192904f,
        -0.0518964f,
        0.188334f,
        0.202665f,
        0.198246f,
        0.279775f,
        0.100275f,
        -0.13811f,
        0.161858f,
        -0.243149f,
        -0.0127482f,
        0.0757492f,
        -0.441334f,
        -0.0268542f,
        -0.114279f,
        -0.177216f,
        -0.0719604f,
        -0.127649f,
        -0.125531f,
        0.520186f,
        -0.0842066f,
        0.236264f,
        0.22663f,
        0.313905f,
        -0.26988f,
        0.188891f,
        -0.273121f,
        -0.339143f,
        0.251947f,
        -0.800452f,
        0.0130017f,
        0.0306637f,
        -0.488503f,
        0.368267f,
        -0.0427116f,
        0.0153203f,
        -0.16417f,
        -0.0864108f,
        0.153452f,
        0.37565f,
        0.431599f,
        0.145022f,
        0.0238288f,
        -0.344559f,
        0.170844f,
        0.220166f,
        0.0584108f,
        -0.0242578f,
        0.459242f,
        -0.309433f,
        -0.183178f,
        0.108402f,
        -0.558459f,
        -0.116711f,
        0.131252f,
        0.0709812f,
        0.158865f,
        -0.222513f,
        -0.0143199f,
        0.179904f,
        -0.325517f,
        0.0144405f,
        -0.248297f,
        0.0659974f,
        0.384588f,
        -0.12348f,
        0.0427167f,
        0.0150143f,
        -0.354596f,
        -0.276234f,
        0.0335361f,
        0.207341f,
        -0.456912f,
        -0.439884f,
        0.0628522f,
        -0.13091f,
        0.199357f,
        -0.218441f,
        -0.367666f,
        -0.0216214f,
        -0.069383f,
        -0.0200709f,
        -0.235768f,
        0.15368f,
        -0.177165f,
        -0.0735893f,
        0.0660757f,
        0.395157f,
        0.0614654f,
        -0.159744f,
        -0.245473f,
        -0.207488f,
        0.253759f,
        -0.0102803f,
        0.116993f,
        -0.0493576f,
        0.0167945f,
        0.138382f,
        -0.00102203f,
        -0.144797f,
        -0.222619f,
        0.320866f,
        0.0507977f,
        -0.0458033f,
        1.10501f,
        -0.116079f,
        -0.393548f,
        -0.320955f,
        -0.386687f,
        0.277903f,
        0.0759589f,
        -0.278561f,
        -0.162686f,
        -0.022386f,
        -0.244537f,
        0.0274179f,
        -0.080937f,
        -0.222133f,
        0.0633341f,
        0.155947f,
        -0.0298876f,
        -0.18921f,
        0.190984f,
        -0.35663f,
        -0.137884f,
        0.550159f,
        0.096353f,
        0.273687f,
        0.288882f,
        0.0811719f,
        0.279464f,
        0.168219f,
        -0.231556f,
        0.338719f,
        0.170307f,
        -0.04944f,
        1.94646f,
        -0.151242f,
        0.192923f,
        0.197884f,
        -0.193047f,
        -0.148041f,
        -0.0801752f,
        0.00753225f,
        -0.0705527f,
        0.10537f,
        -0.212792f,
        0.0661733f,
        0.00248664f,
        0.12624f,
        -0.622626f,
        0.106069f,
        0.0252939f,
        -0.0474646f,
        -0.0415958f,
        -0.207594f,
        -0.0870699f,
        0.0662641f,
        -0.172468f,
        0.152725f,
        0.118974f,
        0.471971f,
        -0.187808f,
        0.0202502f,
        -0.271737f,
        0.395173f,
        0.172049f,
        -0.115912f,
        0.293842f,
        -0.0161143f,
        0.268388f,
        -0.488364f,
        0.405653f,
        -0.318354f,
        0.226183f,
        -0.371718f,
        -0.124455f,
        0.114567f,
        -0.0197764f,
        -0.255166f,
        -0.151114f,
        -0.189752f,
        -0.674399f,
        -0.532053f,
        -0.281676f,
        0.203388f,
        -0.260101f,
        0.207742f,
        -0.16209f,
        0.0490458f,
        0.513714f,
        -0.118339f,
        -0.497515f,
        -0.354277f,
        0.114791f,
        0.147685f,
        0.486502f,
        -0.294311f,
        0.318928f,
        -0.0873725f,
        -0.347909f,
        0.0376368f,
        -0.330455f,
        -0.181159f,
        0.336051f,
        0.332889f,
        -0.0234334f,
        -0.222394f,
        -0.20525f,
        -0.140286f,
        -0.0351668f,
        -0.15004f,
        -0.0625771f,
        -0.0735099f,
        0.0579021f,
        0.353291f,
        -0.416761f,
        0.00784921f,
        0.0550992f,
        0.0785671f,
        0.0327464f,
        0.174488f,
        -0.643194f,
        -0.00167506f,
        -0.18351f,
        -0.77283f,
        -0.141698f,
        -0.283943f,
        0.0703258f,
        0.0731425f,
        0.0893804f,
        0.45594f,
        -0.119529f,
        -0.13004f,
        0.0286476f,
        -0.143618f,
        0.0157178f,
        -0.254329f,
        0.32324f,
        -0.0795797f,
        0.531658f,
        -0.0318883f,
        -0.113471f,
        0.0562344f,
        0.0250965f,
        -0.31784f,
        0.312777f,
        0.217587f,
        0.00470227f,
        0.0759693f,
        -0.244138f,
        0.593975f,
        -0.115257f,
        -0.31279f,
        0.293078f,
        0.0778797f,
        0.105953f,
        0.000129977f,
        0.00625674f,
        0.14876f,
        -0.266717f,
        -0.248158f,
        -0.139808f,
        -0.151966f,
        0.181941f,
        0.178033f,
        -0.0492492f,
        0.346183f,
        0.13544f,
        -0.125893f,
        0.213388f,
        -0.320162f,
        -0.113014f,
        0.1089f,
        -0.0229706f,
        -0.253305f,
        -0.108912f,
        -0.442676f,
        0.194433f,
        0.061225f,
        0.11757f,
        0.16437f,
        0.1215f,
        -0.457864f,
        0.0721076f,
        -0.149889f,
        -0.0636956f,
        0.00125577f,
        -0.0332369f,
        0.0629046f,
        0.111954f,
        0.195584f,
        0.0675324f,
        -0.709152f,
        -0.200995f,
        -0.0563228f,
        -0.217369f,
        0.528605f,
        0.895751f,
        -0.00650362f,
        0.152248f,
        0.349439f,
        -0.0111444f,
        -0.0732277f,
        -0.0588748f,
        0.0367729f,
        -0.0289346f,
        0.317606f,
        0.391562f,
        -0.208744f,
        0.230341f,
        -0.44191f,
        0.0197931f,
        0.709046f,
        -0.224512f,
        -0.641606f,
        -0.0249767f,
        0.0258922f,
        0.0138068f,
        -0.106175f,
        0.0353294f,
        0.402765f,
        0.0908363f,
        -0.114702f,
        0.0693749f,
        0.236688f,
        -0.149579f,
        -0.290914f,
        0.0249069f,
        -0.0761767f,
        0.566583f,
        -0.0756051f,
        -0.0730483f,
        0.60233f,
        -0.0485716f,
        0.445732f,
        -0.652175f,
        0.176092f,
        0.0424668f,
        -0.464009f,
        0.0419261f,
        0.437008f,
        -0.0949722f,
        0.322407f,
        0.00229612f,
        -0.0698273f,
        0.279531f,
        0.0615138f,
        0.029985f,
        0.0397836f,
        -0.0450066f,
        -0.153192f,
        -0.00434645f,
        0.235137f,
        -0.430068f,
        0.0479958f,
        0.0538638f,
        0.0335953f,
        0.059768f,
        -0.00594587f,
        0.313076f,
        0.173102f,
        -0.285881f,
        0.0860142f,
        -0.0656319f,
        0.0903421f,
        0.285596f,
        -0.00867567f,
        0.140828f,
        0.0679629f,
        -0.0386886f,
        -0.0432885f,
        -0.0523634f,
        -0.202516f,
        -0.393785f,
        0.0862689f,
        0.196867f,
        -1.25594f,
        0.407078f,
        0.0883489f,
        -0.145128f,
        -0.0270446f,
        -0.436027f,
        0.124345f,
        0.0216037f,
        -0.0341296f,
        -0.0550939f,
        -0.0143608f,
        0.269727f,
        -0.316092f,
        0.24142f,
        -0.0602843f,
        0.677077f,
        0.015578f,
        -0.0866818f,
        -0.0114968f,
        0.233375f,
        0.0768293f,
        -0.305731f,
        -0.101196f,
        0.116959f,
        -0.705778f,
        -0.274866f,
        0.0159985f,
        -0.0331097f,
        -0.165272f,
        0.00676643f,
        0.194435f,
        -0.644084f,
        -0.0719713f,
        -0.38506f,
        -0.0766179f,
        -0.232665f,
        -0.184962f,
        -0.0873918f,
        0.123643f,
        -0.10653f,
        -0.0842152f,
        0.173415f,
        0.0549799f,
        -0.246653f,
        -0.753266f,
        -0.17178f,
        -0.315154f,
        -0.284002f,
        -0.0944792f,
        0.513692f,
        -0.555574f,
        0.207956f,
        0.108376f,
        -0.322661f,
        -0.181751f,
        0.0175992f,
        -0.067274f,
        -0.0244339f,
        0.264443f,
        -0.307399f,
        0.435229f,
        -0.0912658f,
        0.41608f,
        0.203329f,
        0.0611729f,
        -0.11855f,
        -0.0901752f,
        0.219847f,
        -0.00417572f,
        2.35094f,
        0.0649107f,
        -0.0693629f,
        0.118256f,
        0.0929001f,
        -0.295607f,
        0.102728f,
        0.0786511f,
        -0.164747f,
        0.0778936f,
        0.269988f,
        -0.0515926f,
        0.147636f,
        -0.121939f,
        -0.0303203f,
        -0.702988f,
        -0.220738f,
        -0.104178f,
        0.102754f,
        -0.138891f,
        -0.202057f,
        -0.130212f,
        -0.0471592f,
        0.0441513f,
        -0.0329241f,
        0.124936f,
        0.0593826f,
        0.209008f,
        0.336101f,
        0.360896f,
        0.118351f,
        -0.169321f,
        -0.00791483f,
        -0.584241f,
        -0.507303f,
        -0.103466f,
        0.031976f,
        0.17687f,
        -0.0595649f,
        0.126475f,
        -0.046226f,
        0.143653f,
        -0.0457821f,
        0.0206962f,
        -0.0371797f,
        0.179695f,
        0.1175f,
        -0.551365f,
        0.187117f,
        -0.385674f,
        -0.983871f,
        -0.42516f,
        0.0679211f,
        -0.199898f,
        0.192904f,
        -0.0518964f,
        0.188334f,
        0.202665f,
        0.198246f,
        0.279775f,
        0.100275f,
        -0.13811f,
        0.161858f,
        -0.243149f,
        -0.0127482f,
        0.0757492f,
        -0.441334f,
        -0.0268542f,
        -0.114279f,
        -0.177216f,
        -0.0719604f,
        -0.127649f,
        -0.125531f,
        0.520186f,
        -0.0842066f,
        0.236264f,
        0.22663f,
        0.313905f,
        -0.26988f,
        0.188891f,
        -0.273121f,
        -0.339143f,
        0.251947f,
        -0.800452f,
        0.0130017f,
        0.0306637f,
        -0.488503f,
        0.368267f,
        -0.0427116f,
        0.0153203f,
        -0.16417f,
        -0.0864108f,
        0.153452f,
        0.37565f,
        0.431599f,
        0.145022f,
        0.0238288f,
        -0.344559f,
        0.170844f,
        0.220166f,
        0.0584108f,
        -0.0242578f,
        0.459242f,
        -0.309433f,
        -0.183178f,
        0.108402f,
        -0.558459f,
        -0.116711f,
        0.131252f,
        0.0709812f,
        0.158865f,
        -0.222513f,
        -0.0143199f,
        0.179904f,
        -0.325517f,
        0.0144405f,
        -0.248297f,
        0.0659974f,
        0.384588f,
        -0.12348f,
        0.0427167f,
        0.0150143f,
        -0.354596f,
        -0.276234f,
        0.0335361f,
        0.207341f,
        -0.456912f,
        -0.439884f,
        0.0628522f,
        -0.13091f,
        0.199357f,
        -0.218441f,
        -0.367666f,
        -0.0216214f,
        -0.069383f,
        -0.0200709f,
        -0.235768f,
        0.15368f,
        -0.177165f,
        -0.0735893f,
        0.0660757f,
        0.395157f,
        0.0614654f,
        -0.159744f,
        -0.245473f,
        -0.207488f,
        0.253759f,
        -0.0102803f,
        0.116993f,
        -0.0493576f,
        0.0167945f,
        0.138382f,
        -0.00102203f,
        -0.144797f,
        -0.222619f,
        0.320866f,
        0.0507977f,
        -0.0458033f,
        1.10501f,
        -0.116079f,
        -0.393548f,
        -0.320955f,
        -0.386687f,
        0.277903f,
        0.0759589f,
        -0.278561f,
        -0.162686f,
        -0.022386f,
        -0.244537f,
        0.0274179f,
        -0.080937f,
        -0.222133f,
        0.0633341f,
        0.155947f,
        -0.0298876f,
        -0.18921f,
        0.190984f,
        -0.35663f,
        -0.137884f,
        0.550159f,
        0.096353f,
        0.273687f,
        0.288882f,
        0.0811719f,
        0.279464f,
        0.168219f,
        -0.231556f,
        0.338719f,
        0.170307f,
        -0.04944f,
        1.94646f,
        -0.151242f,
        0.192923f,
        0.197884f,
        -0.193047f,
        -0.148041f,
        -0.0801752f,
        0.00753225f,
        -0.0705527f,
        0.10537f,
        -0.212792f,
        0.0661733f,
        0.00248664f,
        0.12624f,
        -0.622626f,
        0.106069f,
        0.0252939f,
        -0.0474646f,
        -0.0415958f,
        -0.207594f,
        -0.0870699f,
        0.0662641f,
        -0.172468f,
        0.152725f,
        0.118974f,
        0.471971f,
        -0.187808f,
        0.0202502f,
        -0.271737f,
        0.395173f,
        0.172049f,
        -0.115912f,
        0.293842f,
        -0.0161143f,
        0.268388f,
        -0.488364f,
        0.405653f,
        -0.318354f,
        0.226183f,
        -0.371718f,
        -0.124455f,
        0.114567f,
        -0.0197764f,
        -0.255166f,
        -0.151114f,
        -0.189752f,
        -0.674399f,
        -0.532053f,
        -0.281676f,
        0.203388f,
        -0.260101f,
        0.207742f,
        -0.16209f,
        0.0490458f,
        0.513714f,
        -0.118339f,
        -0.497515f,
        -0.354277f,
        0.114791f,
        0.147685f,
        0.486502f,
        -0.294311f,
        0.318928f,
        -0.0873725f,
        -0.347909f,
        0.0376368f,
        -0.330455f,
        -0.181159f,
        0.336051f,
        0.332889f,
        -0.0234334f,
        -0.222394f,
        -0.20525f,
        -0.140286f,
        -0.0351668f,
        -0.15004f,
        -0.0625771f,
        -0.0735099f,
        0.0579021f,
        0.353291f,
        -0.416761f,
        0.00784921f,
        0.0550992f,
        0.0785671f,
        0.0327464f,
        0.174488f,
        -0.643194f,
        -0.00167506f,
        -0.18351f,
        -0.77283f,
        -0.141698f,
        -0.283943f,
        0.0703258f,
        0.0731425f,
        0.0893804f,
        0.45594f,
        -0.119529f,
        -0.13004f,
        0.0286476f,
        -0.143618f,
        0.0157178f,
        -0.254329f,
        0.32324f,
        -0.0795797f,
        0.531658f,
        -0.0318883f,
        -0.113471f,
        0.0562344f,
        0.0250965f,
        -0.31784f,
        0.312777f,
        0.217587f,
        0.00470227f,
        0.0759693f,
        -0.244138f,
        0.593975f,
        -0.115257f,
        -0.31279f,
        0.293078f,
        0.0778797f,
        0.105953f,
        0.000129977f,
        0.00625674f,
        0.14876f,
        -0.266717f,
        -0.248158f,
        -0.139808f,
        -0.151966f,
        0.181941f,
        0.178033f,
        -0.0492492f,
        0.346183f,
        0.13544f,
        -0.125893f,
        0.213388f,
        -0.320162f,
        -0.113014f,
        0.1089f,
        -0.0229706f,
        -0.253305f,
        -0.108912f,
        -0.442676f,
        0.194433f,
        0.061225f,
        0.11757f,
        0.16437f,
        0.1215f,
        -0.457864f,
        0.0721076f,
        -0.149889f,
        -0.0636956f,
        0.00125577f,
        -0.0332369f,
        0.0629046f,
        0.111954f,
        0.195584f,
        0.0675324f,
        -0.709152f,
        -0.200995f,
        -0.0563228f,
        -0.217369f,
        0.528605f,
        0.895751f,
        -0.00650362f,
        0.152248f,
        0.349439f,
        -0.0111444f,
        -0.0732277f,
        -0.0588748f,
        0.0367729f,
        -0.0289346f,
        0.317606f,
        0.391562f,
        -0.208744f,
        0.230341f,
        -0.44191f,
        0.0197931f,
        0.709046f,
        -0.224512f,
        -0.641606f,
        -0.0249767f,
        0.0258922f,
        0.0138068f,
        -0.106175f,
        0.0353294f,
        0.402765f,
        0.0908363f,
        -0.114702f,
        0.0693749f,
        0.236688f,
        -0.149579f,
        -0.290914f,
        0.0249069f,
        -0.0761767f,
        0.566583f,
        -0.0756051f,
        -0.0730483f,
        0.60233f,
        -0.0485716f,
        0.445732f,
        -0.652175f,
        0.176092f,
        0.0424668f,
        -0.464009f,
        0.0419261f,
        0.437008f,
        -0.0949722f,
        0.322407f,
        0.00229612f,
        -0.0698273f,
        0.279531f,
        0.0615138f,
        0.029985f,
        0.0397836f,
        -0.0450066f,
        -0.153192f,
        -0.00434645f,
        0.235137f,
        -0.430068f,
        0.0479958f,
        0.0538638f,
        0.0335953f,
        0.059768f,
        -0.00594587f,
        0.313076f,
        0.173102f,
        -0.285881f,
        0.0860142f,
        -0.0656319f,
        0.0903421f,
        0.285596f,
        -0.00867567f,
        0.140828f,
        0.0679629f,
        -0.0386886f,
        -0.0432885f,
        -0.0523634f,
        -0.202516f,
        -0.393785f,
        0.0862689f,
        0.196867f,
        -1.25594f,
        0.407078f,
        0.0883489f,
        -0.145128f,
        -0.0270446f,
        -0.436027f,
        0.124345f,
        0.0216037f,
        -0.0341296f,
        -0.0550939f,
        -0.0143608f,
        0.269727f,
        -0.316092f,
        0.24142f,
        -0.0602843f,
        0.677077f,
        0.015578f,
        -0.0866818f,
        -0.0114968f,
        0.233375f,
        0.0768293f,
        -0.305731f,
        -0.101196f,
        0.116959f,
        -0.705778f,
        -0.274866f,
        0.0159985f,
        -0.0331097f,
        -0.165272f,
        0.00676643f,
        0.194435f,
        -0.644084f,
        -0.0719713f,
        -0.38506f,
        -0.0766179f,
        -0.232665f,
        -0.184962f,
        -0.0873918f,
        0.123643f,
        -0.10653f,
        -0.0842152f,
        0.173415f,
        0.0549799f,
        -0.246653f,
        -0.753266f,
        -0.17178f,
        -0.315154f,
        -0.284002f,
        -0.0944792f,
        0.513692f,
        -0.555574f,
        0.207956f,
        0.108376f,
        -0.322661f,
        -0.181751f,
        0.0175992f,
        -0.067274f,
        -0.0244339f,
        0.264443f,
        -0.307399f,
        0.435229f,
        -0.0912658f,
        0.41608f,
        0.203329f,
        0.0611729f,
        -0.11855f,
        -0.0901752f,
        0.219847f,
        -0.00417572f,
        2.35094f,
        0.0649107f,
        -0.0693629f,
        0.118256f,
        0.0929001f,
        -0.295607f,
        0.102728f,
        0.0786511f,
        -0.164747f,
        0.0778936f,
        0.269988f,
        -0.0515926f,
        0.147636f,
        -0.121939f,
        -0.0303203f,
        -0.702988f,
        -0.220738f,
        -0.104178f,
        0.102754f,
        -0.138891f,
        -0.202057f,
        -0.130212f,
        -0.0471592f,
        0.0441513f,
        -0.0329241f,
        0.124936f,
        0.0593826f,
        0.209008f,
        0.336101f,
        0.360896f,
        0.118351f,
        -0.169321f,
        -0.00791483f,
        -0.584241f,
        -0.507303f,
        -0.103466f,
        0.031976f,
        0.17687f,
        -0.0595649f,
        0.126475f,
        -0.046226f,
        0.143653f,
        -0.0457821f,
        0.0206962f,
        -0.0371797f,
        0.179695f,
        0.1175f,
        -0.551365f,
        0.187117f,
        -0.385674f,
        -0.983871f,
        -0.42516f,
        0.0679211f,
        -0.199898f,
        0.192904f,
        -0.0518964f,
        0.188334f,
        0.202665f,
        0.198246f,
        0.279775f,
        0.100275f,
        -0.13811f,
        0.161858f,
        -0.243149f,
        -0.0127482f,
        0.0757492f,
        -0.441334f,
        -0.0268542f,
        -0.114279f,
        -0.177216f,
        -0.0719604f,
        -0.127649f,
        -0.125531f,
        0.520186f,
        -0.0842066f,
        0.236264f,
        0.22663f,
        0.313905f,
        -0.26988f,
        0.188891f,
        -0.273121f,
        -0.339143f,
        0.251947f,
        -0.800452f,
        0.0130017f,
        0.0306637f,
        -0.488503f,
        0.368267f,
        -0.0427116f,
        0.0153203f,
        -0.16417f,
        -0.0864108f,
        0.153452f,
        0.37565f,
        0.431599f,
        0.145022f,
        0.0238288f,
        -0.344559f,
        0.170844f,
        0.220166f,
        0.0584108f,
        -0.0242578f,
        0.459242f,
        -0.309433f,
        -0.183178f,
        0.108402f,
        -0.558459f,
        -0.116711f,
        0.131252f,
        0.0709812f,
        0.158865f,
        -0.222513f,
        -0.0143199f,
        0.179904f,
        -0.325517f,
        0.0144405f,
        -0.248297f,
        0.0659974f,
        0.384588f,
        -0.12348f,
        0.0427167f,
        0.0150143f,
        -0.354596f,
        -0.276234f,
        0.0335361f,
        0.207341f,
        -0.456912f,
        -0.439884f,
        0.0628522f,
        -0.13091f,
        0.199357f,
        -0.218441f,
        -0.367666f,
        -0.0216214f,
        -0.069383f,
        -0.0200709f,
        -0.235768f,
        0.15368f,
        -0.177165f,
        -0.0735893f,
        0.0660757f,
        0.395157f,
        0.0614654f,
        -0.159744f,
        -0.245473f,
        -0.207488f,
        0.253759f,
        -0.0102803f,
        0.116993f,
        -0.0493576f,
        0.0167945f,
        0.138382f,
        -0.00102203f,
        -0.144797f,
        -0.222619f,
        0.320866f,
        0.0507977f,
        -0.0458033f,
        1.10501f,
        -0.116079f,
        -0.393548f,
        -0.320955f,
        -0.386687f,
        0.277903f,
        0.0759589f,
        -0.278561f,
        -0.162686f,
        -0.022386f,
        -0.244537f,
        0.0274179f,
        -0.080937f,
        -0.222133f,
        0.0633341f,
        0.155947f,
        -0.0298876f,
        -0.18921f,
        0.190984f,
        -0.35663f,
        -0.137884f,
        0.550159f,
        0.096353f,
        0.273687f,
        0.288882f,
        0.0811719f,
        0.279464f,
        0.168219f,
        -0.231556f,
        0.338719f,
        0.170307f,
        -0.04944f,
        1.94646f,
        -0.151242f,
        0.192923f,
        0.197884f,
        -0.193047f,
        -0.148041f,
        -0.0801752f,
        0.00753225f,
        -0.0705527f,
        0.10537f,
        -0.212792f,
        0.0661733f,
        0.00248664f,
        0.12624f,
        -0.622626f,
        0.106069f,
        0.0252939f,
        -0.0474646f,
        -0.0415958f,
        -0.207594f,
        -0.0870699f,
        0.0662641f,
        -0.172468f,
        0.152725f,
        0.118974f,
        0.471971f,
        -0.187808f,
        0.0202502f,
        -0.271737f,
        0.395173f,
        0.172049f,
        -0.115912f,
        0.293842f,
        -0.0161143f,
        0.268388f,
        -0.488364f,
        0.405653f,
        -0.318354f,
        0.226183f,
        -0.371718f,
        -0.124455f,
        0.114567f,
        -0.0197764f,
        -0.255166f,
        -0.151114f,
        -0.189752f,
        -0.674399f,
        -0.532053f,
        -0.281676f,
        0.203388f,
        -0.260101f,
        0.207742f,
        -0.16209f,
        0.0490458f,
        0.513714f,
        -0.118339f,
        -0.497515f,
        -0.354277f,
        0.114791f,
        0.147685f,
        0.486502f,
        -0.294311f,
        0.318928f,
        -0.0873725f,
        -0.347909f,
        0.0376368f,
        -0.330455f,
        -0.181159f,
        0.336051f,
        0.332889f,
        -0.0234334f,
        -0.222394f,
        -0.20525f,
        -0.140286f,
        -0.0351668f,
        -0.15004f,
        -0.0625771f,
        -0.0735099f,
        0.0579021f,
        0.353291f,
        -0.416761f,
        0.00784921f,
        0.0550992f,
        0.0785671f,
        0.0327464f,
        0.174488f,
        -0.643194f,
        -0.00167506f,
        -0.18351f,
        -0.77283f,
        -0.141698f,
        -0.283943f,
        0.0703258f,
        0.0731425f,
        0.0893804f,
        0.45594f,
        -0.119529f,
        -0.13004f,
        0.0286476f,
        -0.143618f,
        0.0157178f,
        -0.254329f,
        0.32324f,
        -0.0795797f,
        0.531658f,
        -0.0318883f,
        -0.113471f,
        0.0562344f,
        0.0250965f,
        -0.31784f,
        0.312777f,
        0.217587f,
        0.00470227f,
        0.0759693f,
        -0.244138f,
        0.593975f,
        -0.115257f,
        -0.31279f,
        0.293078f,
        0.0778797f,
        0.105953f,
        0.000129977f,
        0.00625674f,
        0.14876f,
        -0.266717f,
        -0.248158f,
        -0.139808f,
        -0.151966f,
        0.181941f,
        0.178033f,
        -0.0492492f,
        0.346183f,
        0.13544f,
        -0.125893f,
        0.213388f,
        -0.320162f,
        -0.113014f,
        0.1089f,
        -0.0229706f,
        -0.253305f,
        -0.108912f,
        -0.442676f,
        0.194433f,
        0.061225f,
        0.11757f,
        0.16437f,
        0.1215f,
        -0.457864f,
        0.0721076f,
        -0.149889f,
        -0.0636956f,
        0.00125577f,
        -0.0332369f,
        0.0629046f,
        0.111954f,
        0.195584f,
        0.0675324f,
        -0.709152f,
        -0.200995f,
        -0.0563228f,
        -0.217369f,
        0.528605f,
        0.895751f,
        -0.00650362f,
        0.152248f,
        0.349439f,
        -0.0111444f,
        -0.0732277f,
        -0.0588748f,
        0.0367729f,
        -0.0289346f,
        0.317606f,
        0.391562f,
        -0.208744f,
        0.230341f,
        -0.44191f,
        0.0197931f,
        0.709046f,
        -0.224512f,
        -0.641606f,
        -0.0249767f,
        0.0258922f,
        0.0138068f,
        -0.106175f,
        0.0353294f,
        0.402765f,
        0.0908363f,
        -0.114702f,
        0.0693749f,
        0.236688f,
        -0.149579f,
        -0.290914f,
        0.0249069f,
        -0.0761767f,
        0.566583f,
        -0.0756051f,
        -0.0730483f,
        0.60233f,
        -0.0485716f,
        0.445732f,
        -0.652175f,
        0.176092f,
        0.0424668f,
        -0.464009f,
        0.0419261f,
        0.437008f,
        -0.0949722f,
        0.322407f,
        0.00229612f,
        -0.0698273f,
        0.279531f,
        0.0615138f,
        0.029985f,
        0.0397836f,
        -0.0450066f,
        -0.153192f,
        -0.00434645f,
        0.235137f,
        -0.430068f,
        0.0479958f,
        0.0538638f,
        0.0335953f,
        0.059768f,
        -0.00594587f,
        0.313076f,
        0.173102f,
        -0.285881f,
        0.0860142f,
        -0.0656319f,
        0.0903421f,
        0.285596f,
        -0.00867567f,
        0.140828f,
        0.0679629f,
        -0.0386886f,
        -0.0432885f,
        -0.0523634f,
        -0.202516f,
        -0.393785f,
        0.0862689f,
        0.196867f,
        -1.25594f,
        0.407078f,
        0.0883489f,
        -0.145128f,
        -0.0270446f,
        -0.436027f,
        0.124345f,
        0.0216037f,
        -0.0341296f,
        -0.0550939f,
        -0.0143608f,
        0.269727f,
        -0.316092f,
        0.24142f,
        -0.0602843f,
        0.677077f,
        0.015578f,
        -0.0866818f,
        -0.0114968f,
        0.233375f,
        0.0768293f,
        -0.305731f,
        -0.101196f,
        0.116959f,
        -0.705778f,
        -0.274866f,
        0.0159985f,
        -0.0331097f,
        -0.165272f,
        0.00676643f,
        0.194435f,
        -0.644084f,
        -0.0719713f,
        -0.38506f,
        -0.0766179f,
        -0.232665f,
        -0.184962f,
        -0.0873918f,
        0.123643f,
        -0.10653f,
        -0.0842152f,
        0.173415f,
        0.0549799f,
        -0.246653f,
        -0.753266f,
        -0.17178f,
        -0.315154f,
        -0.284002f,
        -0.0944792f,
        0.513692f,
        -0.555574f,
        0.207956f,
        0.108376f,
        -0.322661f,
        -0.181751f,
        0.0175992f,
        -0.067274f,
        -0.0244339f,
        0.264443f,
        -0.307399f,
        0.435229f,
        -0.0912658f,
        0.41608f,
        0.203329f,
        0.0611729f,
        -0.11855f,
        -0.0901752f,
        0.219847f,
        -0.00417572f,
        2.35094f,
        0.0649107f,
        -0.0693629f,
        0.118256f,
        0.0929001f,
        -0.295607f,
        0.102728f,
        0.0786511f,
        -0.164747f,
        0.0778936f,
        0.269988f,
        -0.0515926f,
        0.147636f,
        -0.121939f,
        -0.0303203f,
        -0.702988f,
        -0.220738f,
        -0.104178f,
        0.102754f,
        -0.138891f,
        -0.202057f,
        -0.130212f,
        -0.0471592f,
        0.0441513f,
        -0.0329241f,
        0.124936f,
        0.0593826f,
        0.209008f,
        0.336101f,
        0.360896f,
        0.118351f,
        -0.169321f,
        -0.00791483f,
        -0.584241f,
        -0.507303f,
        -0.103466f,
        0.031976f,
        0.17687f,
        -0.0595649f,
        0.126475f,
        -0.046226f,
        0.143653f,
        -0.0457821f,
        0.0206962f,
        -0.0371797f,
        0.179695f,
        0.1175f,
        -0.551365f,
        0.187117f,
        -0.385674f,
        -0.983871f,
        -0.42516f,
        0.0679211f,
        -0.199898f,
        0.192904f,
        -0.0518964f,
        0.188334f,
        0.202665f,
        0.198246f,
        0.279775f,
        0.100275f,
        -0.13811f,
        0.161858f,
        -0.243149f,
        -0.0127482f,
        0.0757492f,
        -0.441334f,
        -0.0268542f,
        -0.114279f,
        -0.177216f,
        -0.0719604f,
        -0.127649f,
        -0.125531f,
        0.520186f,
        -0.0842066f,
        0.236264f,
        0.22663f,
        0.313905f,
        -0.26988f,
        0.188891f,
        -0.273121f,
        -0.339143f,
        0.251947f,
        -0.800452f,
        0.0130017f,
        0.0306637f,
        -0.488503f,
        0.368267f,
        -0.0427116f,
        0.0153203f,
        -0.16417f,
        -0.0864108f,
        0.153452f,
        0.37565f,
        0.431599f,
        0.145022f,
        0.0238288f,
        -0.344559f,
        0.170844f,
        0.220166f,
        0.0584108f,
        -0.0242578f,
        0.459242f,
        -0.309433f,
        -0.183178f,
        0.108402f,
        -0.558459f,
        -0.116711f,
        0.131252f,
        0.0709812f,
        0.158865f,
        -0.222513f,
        -0.0143199f,
        0.179904f,
        -0.325517f,
        0.0144405f,
        -0.248297f,
        0.0659974f,
        0.384588f,
        -0.12348f,
        0.0427167f,
        0.0150143f,
        -0.354596f,
        -0.276234f,
        0.0335361f,
        0.207341f,
        -0.456912f,
        -0.439884f,
        0.0628522f,
        -0.13091f,
        0.199357f,
        -0.218441f,
        -0.367666f,
        -0.0216214f,
        -0.069383f,
        -0.0200709f,
        -0.235768f,
        0.15368f,
        -0.177165f,
        -0.0735893f,
        0.0660757f,
        0.395157f,
        0.0614654f,
        -0.159744f,
        -0.245473f,
        -0.207488f,
        0.253759f,
        -0.0102803f,
        0.116993f,
        -0.0493576f,
        0.0167945f,
        0.138382f,
        -0.00102203f,
        -0.144797f,
        -0.222619f,
        0.320866f,
        0.0507977f,
        -0.0458033f,
        1.10501f,
        -0.116079f,
        -0.393548f,
        -0.320955f,
        -0.386687f,
        0.277903f,
        0.0759589f,
        -0.278561f,
        -0.162686f,
        -0.022386f,
        -0.244537f,
        0.0274179f,
        -0.080937f,
        -0.222133f,
        0.0633341f,
        0.155947f,
        -0.0298876f,
        -0.18921f,
        0.190984f,
        -0.35663f,
        -0.137884f,
        0.550159f,
        0.096353f,
        0.273687f,
        0.288882f,
        0.0811719f,
        0.279464f,
        0.168219f,
        -0.231556f,
        0.338719f,
        0.170307f,
        -0.04944f,
        1.94646f,
        -0.151242f,
        0.192923f,
        0.197884f,
        -0.193047f,
        -0.148041f,
        -0.0801752f,
        0.00753225f,
        -0.0705527f,
        0.10537f,
        -0.212792f,
        0.0661733f,
        0.00248664f,
        0.12624f,
        -0.622626f,
        0.106069f,
        0.0252939f,
        -0.0474646f,
        -0.0415958f,
        -0.207594f,
        -0.0870699f,
        0.0662641f,
        -0.172468f,
        0.152725f,
        0.118974f,
        0.471971f,
        -0.187808f,
        0.0202502f,
        -0.271737f,
        0.395173f,
        0.172049f,
        -0.115912f,
        0.293842f,
        -0.0161143f,
        0.268388f,
        -0.488364f,
        0.405653f,
        -0.318354f,
        0.226183f,
        -0.371718f,
        -0.124455f,
        0.114567f,
        -0.0197764f,
        -0.255166f,
        -0.151114f,
        -0.189752f,
        -0.674399f,
        -0.532053f,

    };
    set_values(input, input_data);
    set_values(input2, input_data2);

    std::vector<float> out_data = {
        1.4538f,  0.1023f,  0.9402f,  0.1373f, -0.5619f, -0.6068f, -1.4220f,
        -0.8573f,  0.9406f,

        1.3968f, -0.0450f,  0.7518f,  0.4434f, -0.2937f, -0.4165f, -0.8629f,
        -0.5942f,  0.4324f,

        1.3968f, -0.0450f,  0.7518f,  0.4434f, -0.2937f, -0.4165f, -0.8629f,
        -0.5942f,  0.4324f,

        1.3968f, -0.0450f,  0.7518f,  0.4434f, -0.2937f, -0.4165f, -0.8629f,
        -0.5942f,  0.4324f,

        1.3968f, -0.0450f,  0.7518f,  0.4434f, -0.2937f, -0.4165f, -0.8629f,
        -0.5942f,  0.4324f

    };

    topology topology;
    topology.add(
        input_layout("input", input->get_layout())
    );
    topology.add(
        input_layout("input2", input2->get_layout())
    );
    topology.add(
        gemm("output", { "input", "input2" }, data_types::f32)
    );

    network network(engine, topology);
    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    auto outputs = network.execute();

    auto output = outputs.at("output").get_memory();
    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    EXPECT_EQ(output_ptr.size(), (uint32_t)45);
    for (uint32_t  i = 0; i < out_data.size(); ++i) {
        EXPECT_NEAR(output_ptr[i], out_data[i], 0.0001);
    }

}

TEST(gemm_gpu, basic_smarcink2) {
    auto& engine = get_test_engine();
    auto input = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 1, 3, 2 } });
    auto input2 = engine.allocate_memory({ data_types::f32, format::bfyx, { 2, 1, 2, 3 } });

    std::vector<float> input_data = {
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,

        1.0f, 1.0f, 1.0f,
        2.0f, 3.0f, 4.0f
    };

    std::vector<float> input_data2 = {
        7.0f, 8.0f,
        9.0f, 10.0f,
        11.0f, 12.0f,

        1.0f, 1.0f,
        1.0f, 1.0f,
        3.0f, 2.0f
    };
    set_values(input, input_data);
    set_values(input2, input_data2);

    std::vector<float> out_data = {
        58.0f, 64.0f,
        139.0f, 154.0f,

        5.0f, 4.0f,
        17.0f, 13.0f
    };

    topology topology;
    topology.add(
        input_layout("input", input->get_layout())
    );
    topology.add(
        input_layout("input2", input2->get_layout())
    );
    topology.add(
        gemm("output", { "input", "input2" }, data_types::f32)
    );

    network network(engine, topology);
    network.set_input_data("input", input);
    network.set_input_data("input2", input2);
    auto outputs = network.execute();

    auto output = outputs.at("output").get_memory();
    cldnn::mem_lock<float> output_ptr(output, get_test_stream());

    EXPECT_EQ(output_ptr.size(), (uint32_t)8);
    for (uint32_t i = 0; i < out_data.size(); ++i) {
        EXPECT_FLOAT_EQ(output_ptr[i], out_data[i]);
    }
}

struct gemm_base_test_params {
    size_t m_size;
    size_t n_size;
    size_t k_size;
    size_t b0_num;
    size_t f0_num;
    size_t b1_num;
    size_t f1_num;
    size_t b2_num;
    size_t f2_num;
    size_t b_out_num;
    size_t f_out_num;
    bool transpose_input0;
    bool transpose_input1;
    float alpha;
    float beta;
    cldnn::data_types allocate0_type;
    cldnn::data_types allocate1_type;
    cldnn::data_types allocate2_type;
    cldnn::data_types output_type;
    std::vector <int> range0;
    std::vector <int> range1;
    std::vector <int> range2;
    std::string kernel_name;
};

#ifdef ENABLE_ONEDNN_FOR_GPU

#define CASE_GEMM_INT8_ONEDNN_1 1, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 0.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_ONEDNN_2 64, 1, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 0.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_ONEDNN_3 1, 1, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 0.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_ONEDNN_4 64, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 0.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }

#define CASE_GEMM_UINT8_ONEDNN_1 1, 64, 64, 2, 2, 2, 2, 2, 2, 2, 2, false, false, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_UINT8_ONEDNN_2 64, 1, 64, 2, 2, 2, 2, 2, 2, 2, 2, false, false, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_UINT8_ONEDNN_3 1, 1, 64, 2, 2, 2, 2, 2, 2, 2, 2, false, false, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_UINT8_ONEDNN_4 64, 64, 64, 2, 2, 2, 2, 2, 2, 2, 2, false, false, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }

#define CASE_GEMM_FP16_ONEDNN_1 1, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_ONEDNN_2 64, 1, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_ONEDNN_3 1, 1, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_ONEDNN_4 64, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }

#define CASE_GEMM_FP32_ONEDNN_1 1, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_ONEDNN_2 64, 1, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_ONEDNN_3 1, 1, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_ONEDNN_4 64, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }

#define CASE_GEMM_INT8_NN_TRANSPOSITION_ONEDNN 64, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 0.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_NT_TRANSPOSITION_ONEDNN 64, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.0f, 0.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_TN_TRANSPOSITION_ONEDNN 64, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.0f, 0.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_TT_TRANSPOSITION_ONEDNN 64, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.0f, 0.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }

#define CASE_GEMM_INT8_NN_TRANSPOSITION_LEFTOVERS_ONEDNN 13, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 0.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_NT_TRANSPOSITION_LEFTOVERS_ONEDNN 13, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.0f, 0.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_TN_TRANSPOSITION_LEFTOVERS_ONEDNN 13, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.0f, 0.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_TT_TRANSPOSITION_LEFTOVERS_ONEDNN 13, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.0f, 0.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }

#define CASE_GEMM_UINT8_NN_TRANSPOSITION_ONEDNN 64, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_UINT8_NT_TRANSPOSITION_ONEDNN 64, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_UINT8_TN_TRANSPOSITION_ONEDNN 64, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_UINT8_TT_TRANSPOSITION_ONEDNN 64, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }

#define CASE_GEMM_UINT8_NN_TRANSPOSITION_LEFTOVERS_ONEDNN 13, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_UINT8_NT_TRANSPOSITION_LEFTOVERS_ONEDNN 13, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_UINT8_TN_TRANSPOSITION_LEFTOVERS_ONEDNN 13, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_UINT8_TT_TRANSPOSITION_LEFTOVERS_ONEDNN 13, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }

#define CASE_GEMM_FP16_NN_TRANSPOSITION_ONEDNN 32, 16, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_NT_TRANSPOSITION_ONEDNN 16, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.0f, 1.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TN_TRANSPOSITION_ONEDNN 32, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.0f, 1.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TT_TRANSPOSITION_ONEDNN 32, 64, 96, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.0f, 1.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }

#define CASE_GEMM_FP32_NN_TRANSPOSITION_ONEDNN 32, 16, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32,  { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_NT_TRANSPOSITION_ONEDNN 16, 64, 128, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.0f, 1.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32,  { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TN_TRANSPOSITION_ONEDNN 32, 64, 96, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.0f, 1.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32,  { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TT_TRANSPOSITION_ONEDNN 32, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.0f, 1.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32,  { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }

#define CASE_GEMM_INT8_BROADCASTING_ONEDNN_1 32, 32, 64, 1, 2, 1, 1, 1, 1, 1, 2, false, false, \
1.0f, 1.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_BROADCASTING_ONEDNN_2 32, 32, 64, 2, 1, 1, 1, 1, 1, 2, 1, false, false, \
1.0f, 1.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_BROADCASTING_ONEDNN_3 64, 32, 64, 1, 2, 1, 1, 1, 2, 1, 2, false, false, \
1.0f, 1.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_BROADCASTING_ONEDNN_4 32, 64, 64, 1, 1, 2, 2, 2, 2, 2, 2, false, false, \
1.0f, 1.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }

#define CASE_GEMM_FP16_BROADCASTING_ONEDNN_1 32, 32, 64, 1, 2, 1, 1, 1, 1, 1, 2, false, false, \
1.0f, 0.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_BROADCASTING_ONEDNN_2 32, 32, 64, 2, 1, 1, 1, 1, 1, 2, 1, false, false, \
1.0f, 0.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_BROADCASTING_ONEDNN_3 64, 32, 64, 1, 2, 1, 1, 1, 2, 1, 2, false, false, \
1.0f, 0.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_BROADCASTING_ONEDNN_4 32, 64, 64, 1, 1, 2, 2, 2, 2, 2, 2, false, false, \
1.0f, 0.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }

#define CASE_GEMM_FP32_BROADCASTING_ONEDNN_1 32, 32, 64, 1, 2, 1, 1, 1, 1, 1, 2, false, false, \
1.0f, 0.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_BROADCASTING_ONEDNN_2 32, 32, 64, 2, 1, 1, 1, 1, 1, 2, 1, false, false, \
1.0f, 0.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_BROADCASTING_ONEDNN_3 64, 32, 64, 1, 2, 1, 1, 1, 2, 1, 2, false, false, \
1.0f, 0.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_BROADCASTING_ONEDNN_4 32, 64, 64, 1, 1, 2, 2, 2, 2, 2, 2, false, false, \
1.0f, 0.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }

#define CASE_GEMM_INT8_COMBO_ONEDNN_1 5, 18, 99, 1, 2, 1, 1, 1, 1, 1, 2, false, false, \
1.0f, 1.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_COMBO_ONEDNN_2 1, 32, 65, 2, 1, 1, 1, 1, 1, 2, 1, false, true, \
1.0f, 1.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_COMBO_ONEDNN_3 13, 4, 64, 1, 2, 1, 1, 1, 2, 1, 2, true, false, \
1.0f, 1.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_COMBO_ONEDNN_4 128, 126, 127, 1, 1, 2, 2, 2, 2, 2, 2, true, true, \
1.0f, 1.0f, data_types::i8, data_types::i8, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }

#define CASE_GEMM_UINT8_COMBO_ONEDNN_1 11, 16, 65, 1, 2, 1, 1, 1, 1, 1, 2, false, false, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_UINT8_COMBO_ONEDNN_2 13, 14, 64, 2, 1, 1, 1, 1, 1, 2, 1, false, true, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_UINT8_COMBO_ONEDNN_3 16, 16, 99, 1, 2, 1, 2, 1, 2, 1, 2, true, false, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_UINT8_COMBO_ONEDNN_4 3, 1, 77, 1, 1, 2, 2, 2, 2, 2, 2, true, true, \
1.0f, 1.0f, data_types::u8, data_types::i8, data_types::f32, data_types::f32, { 0, 255, 1 }, { -128, 127, 1 }, { -10, 10, 8 }

// Currently broadcasting support wasn't implemented for f16 cases with biases
#define CASE_GEMM_FP16_COMBO_ONEDNN_1 5, 7, 65, 1, 2, 1, 1, 1, 1, 1, 2, false, false, \
1.0f, 0.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_COMBO_ONEDNN_2 32, 8, 128, 2, 1, 1, 1, 1, 1, 2, 1, false, true, \
1.0f, 0.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_COMBO_ONEDNN_3 14, 2, 69, 1, 2, 1, 1, 1, 2, 1, 2, true, false, \
1.0f, 0.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_COMBO_ONEDNN_4 1, 1, 64, 1, 1, 2, 2, 2, 2, 2, 2, true, true, \
1.0f, 0.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }

#define CASE_GEMM_FP32_COMBO_ONEDNN_1 7, 17, 64, 1, 2, 1, 1, 1, 1, 1, 2, false, false, \
1.0f, 1.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_COMBO_ONEDNN_2 26, 22, 79, 2, 1, 1, 1, 1, 1, 2, 1, false, true, \
1.0f, 1.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_COMBO_ONEDNN_3 5, 7, 81, 1, 2, 1, 1, 1, 2, 1, 2, true, false, \
1.0f, 1.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_COMBO_ONEDNN_4 61, 1, 99, 1, 1, 2, 2, 2, 2, 2, 2, true, true, \
1.0f, 1.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -128, 127, 1 }, { -128, 127, 1 }, { -10, 10, 8 }

#else // ENABLE_ONEDNN_FOR_GPU

#define CASE_GEMM_INT8_NN_TRANSPOSITION 64, 64, 64, 1, 2, 1, 2, 1, 2, 1, 2, false, false, \
1.5f, 2.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_NT_TRANSPOSITION 32, 64, 32, 2, 1, 2, 1, 2, 1, 2, 1, false, true, \
1.7f, 1.3f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_TN_TRANSPOSITION 128, 64, 32, 2, 2, 2, 2, 2, 2, 2, 2, true, false, \
1.0f, 0.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_TT_TRANSPOSITION 32, 32, 32, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.2f, 0.5f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }

#define CASE_GEMM_INT8_BROADCAST_1 32, 32, 32, 1, 2, 1, 1, 1, 1, 1, 2, false, false, \
1.5f, 2.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_BROADCAST_2 32, 32, 64, 2, 1, 1, 1, 1, 1, 2, 1, false, false, \
1.7f, 1.3f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_BROADCAST_3 64, 32, 32, 1, 2, 2, 1, 1, 2, 2, 2, false, false, \
1.0f, 1.5f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_BROADCAST_4 32, 64, 32, 1, 1, 2, 2, 2, 2, 2, 2, false, false, \
1.2f, 0.5f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }

#define CASE_GEMM_INT8_LEFTOVERS_1 13, 32, 32, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.5f, 2.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_LEFTOVERS_2 13, 32, 32, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.6f, 1.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_LEFTOVERS_3 13, 32, 32, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.0f, 1.5f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_LEFTOVERS_4 13, 32, 32, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.7f, 1.3f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_LEFTOVERS_5 32, 13, 32, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.5f, 2.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_LEFTOVERS_6 32, 13, 32, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.6f, 1.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_LEFTOVERS_7 32, 13, 32, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.0f, 1.5f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_LEFTOVERS_8 32, 13, 32, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.7f, 1.3f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_LEFTOVERS_9 32, 32, 13, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.5f, 2.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_LEFTOVERS_10 32, 32, 13, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.6f, 1.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_LEFTOVERS_11 32, 32, 13, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.0f, 1.5f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_LEFTOVERS_12 32, 32, 13, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.7f, 1.3f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }

#define CASE_GEMM_INT8_COMBO_1 8, 8, 32, 1, 2, 1, 1, 1, 1, 1, 2, false, false, \
1.5f, 2.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_COMBO_2 16, 16, 64, 2, 1, 1, 1, 1, 1, 2, 1, false, true, \
1.7f, 0.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_COMBO_3 11, 31, 21, 7, 15, 7, 15, 7, 15, 7, 15, true, false, \
1.0f, 1.5f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_COMBO_4 32, 32, 32, 3, 6, 3, 6, 3, 6, 3, 6, true, true, \
1.2f, 4.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }

#define CASE_GEMM_INT8_SLM_COMBO_1 64, 64, 64, 1, 2, 1, 1, 1, 1, 1, 2, false, false, \
1.5f, 2.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_SLM_COMBO_2 384, 384, 64, 2, 1, 1, 1, 1, 1, 2, 1, false, false, \
1.7f, 0.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_SLM_COMBO_3 128, 128, 64, 2, 3, 2, 3, 2, 3, 2, 3, false, false, \
1.0f, 1.5f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }
#define CASE_GEMM_INT8_SLM_COMBO_4 256, 64, 64, 3, 6, 3, 6, 3, 6, 3, 6, false, false, \
1.2f, 4.0f, data_types::i8, data_types::u8, data_types::f32, data_types::f32, { -128, 127, 1 }, { 0, 255, 1 }, { -10, 10, 8 }

#define CASE_GEMM_FP32_TILED_NN_1 32, 32, 32, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.5f, 2.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_NN_2 64, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.7f, 0.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_NN_3 31, 47, 65, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.5f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_NN_4 65, 31, 47, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 4.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }

#define CASE_GEMM_FP32_TILED_NT_1 16, 16, 16, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.5f, 2.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_NT_2 32, 32, 32, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.7f, 0.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_NT_3 64, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.0f, 1.5f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_NT_4 16, 128, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.0f, 4.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }

#define CASE_GEMM_FP32_TILED_TN_1 16, 16, 16, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.5f, 2.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_TN_2 32, 32, 32, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.7f, 0.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_TN_3 64, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.0f, 1.5f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_TN_4 16, 128, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.0f, 4.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }

#define CASE_GEMM_FP32_TILED_TT_1 16, 16, 16, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.5f, 2.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_TT_2 32, 32, 32, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.7f, 0.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_TT_3 64, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.0f, 1.5f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_TT_4 16, 128, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.0f, 4.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }

#define CASE_GEMM_FP32_TILED_NN_BROADCAST_1 64, 96, 32, 1, 2, 1, 1, 1, 1, 1, 2, false, false, \
1.5f, 2.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_NN_BROADCAST_2 32, 16, 16, 2, 1, 1, 1, 1, 1, 2, 1, false, false, \
1.7f, 0.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_NN_BROADCAST_3 5, 1, 3, 1, 2, 2, 1, 1, 2, 2, 2, false, false, \
1.0f, 1.5f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }
#define CASE_GEMM_FP32_TILED_NN_BROADCAST_4 64, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, false, false, \
1.0f, 4.0f, data_types::f32, data_types::f32, data_types::f32, data_types::f32, { -10, 10, 8 }, { -10, 10, 8 }, { -10, 10, 8 }

#define CASE_GEMM_FP16_TILED_NN_1 64, 32, 32, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.5f, 2.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_NN_2 128, 64, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.7f, 0.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_NN_3 131, 17, 15, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 1.5f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_NN_4 33, 17, 17, 1, 1, 1, 1, 1, 1, 1, 1, false, false, \
1.0f, 4.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }

#define CASE_GEMM_FP16_TILED_NT_1 16, 16, 16, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.5f, 2.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_NT_2 32, 32, 32, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.7f, 0.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_NT_3 64, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.0f, 1.5f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_NT_4 16, 128, 64, 1, 1, 1, 1, 1, 1, 1, 1, false, true, \
1.0f, 4.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }

#define CASE_GEMM_FP16_TILED_TN_1 16, 16, 16, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.5f, 2.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_TN_2 32, 32, 32, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.7f, 0.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_TN_3 64, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.0f, 1.5f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_TN_4 16, 128, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, false, \
1.0f, 4.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }

#define CASE_GEMM_FP16_TILED_TT_1 16, 16, 16, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.5f, 2.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_TT_2 32, 32, 32, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.7f, 0.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_TT_3 64, 32, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.0f, 1.5f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_TT_4 16, 128, 64, 1, 1, 1, 1, 1, 1, 1, 1, true, true, \
1.0f, 4.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }

#define CASE_GEMM_FP16_TILED_NN_BROADCAST_1 64, 96, 128, 1, 2, 1, 1, 1, 1, 1, 2, false, false, \
1.5f, 2.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_NN_BROADCAST_2 64, 16, 64, 2, 1, 1, 1, 1, 1, 2, 1, false, false, \
1.7f, 0.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_NN_BROADCAST_3 1, 2, 3, 1, 2, 2, 1, 1, 2, 2, 2, false, false, \
1.0f, 1.5f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }
#define CASE_GEMM_FP16_TILED_NN_BROADCAST_4 8, 8, 8, 1, 1, 2, 2, 2, 2, 2, 2, false, false, \
1.0f, 4.0f, data_types::f16, data_types::f16, data_types::f16, data_types::f16, { -1, 1, 1 }, { -1, 1, 1 }, { -1, 1, 1 }

#endif // ENABLE_ONEDNN_FOR_GPU

template <typename gemm_params, typename input0_type, typename input1_type, typename input2_type, typename output_type, typename accumulator_type>
class GemmBaseTest : public ::testing::TestWithParam<gemm_params> {
public:

    inline size_t getGemmIndex(size_t x, size_t y, size_t f, size_t b, size_t x_size, size_t y_size, size_t f_num, size_t b_num,
                               size_t x_pitch, size_t y_pitch, size_t f_pitch, size_t b_pitch) {
        return (x % x_size) * x_pitch + (y % y_size) * y_pitch + (f % f_num) * f_pitch + (b % b_num) * b_pitch;
    }

    void execute(gemm_params& p) {
#ifdef ENABLE_ONEDNN_FOR_GPU
        auto& engine = get_onednn_test_engine();
        if (!engine.get_device_info().supports_immad)
            return;
#else
        auto& engine = get_test_engine();
#endif
        auto y0_size = p.m_size;
        auto y0_pitch = p.k_size;
        auto x0_size = p.k_size;
        auto x0_pitch = 1;
        auto f0_pitch = y0_size * x0_size;
        auto b0_pitch = p.f0_num * f0_pitch;

        auto y1_size = p.k_size;
        auto y1_pitch = p.n_size;
        auto x1_size = p.n_size;
        auto x1_pitch = 1;
        auto f1_pitch = y1_size * x1_size;
        auto b1_pitch = p.f1_num * f1_pitch;

        auto y2_size = p.m_size;
        auto y2_pitch = p.n_size;
        auto x2_size = p.n_size;
        auto x2_pitch = 1;
        auto f2_pitch = y2_size * x2_size;
        auto b2_pitch = p.f2_num * f2_pitch;

        auto y_out_size = p.m_size;
        auto y_out_pitch = p.n_size;
        auto x_out_size = p.n_size;
        auto x_out_pitch = 1;
        auto f_out_pitch = y_out_size * x_out_size;
        auto b_out_pitch = p.f_out_num * f_out_pitch;

        if (p.transpose_input0) {
            y0_size = p.k_size;
            y0_pitch = p.m_size;
            x0_size = p.m_size;
            x0_pitch = 1;
        }

        if (p.transpose_input1) {
            y1_size = p.n_size;
            y1_pitch = p.k_size;
            x1_size = p.k_size;
            x1_pitch = 1;
        }

        auto input0_size = tensor((int)p.b0_num, (int)p.f0_num, (int)x0_size, (int)y0_size);
        VVVVF<input0_type> input0_data = generate_random_4d<input0_type>(p.b0_num, p.f0_num, x0_size, y0_size, p.range0[0], p.range0[1], p.range0[2]);
        auto input0_data_bfyx = flatten_4d(format::bfyx, input0_data);
        auto input0_mem = engine.allocate_memory({ p.allocate0_type, format::bfyx, input0_size });
        set_values(input0_mem, input0_data_bfyx);

        auto input1_size = tensor((int)p.b1_num, (int)p.f1_num, (int)x1_size, (int)y1_size);
        VVVVF<input1_type> input1_data = generate_random_4d<input1_type>(p.b1_num, p.f1_num, x1_size, y1_size, p.range1[0], p.range1[1], p.range1[2]);
        auto input1_data_bfyx = flatten_4d(format::bfyx, input1_data);
        auto input1_mem = engine.allocate_memory({ p.allocate1_type, format::bfyx, input1_size });
        set_values(input1_mem, input1_data_bfyx);

        auto input2_size = tensor((int)p.b2_num, (int)p.f2_num, (int)x2_size, (int)y2_size);
        VVVVF<input2_type> input2_data = generate_random_4d<input2_type>(p.b2_num, p.f2_num, x2_size, y2_size, p.range2[0], p.range2[1], p.range2[2]);
        auto input2_data_bfyx = flatten_4d(format::bfyx, input2_data);
        auto input2_mem = engine.allocate_memory({ p.allocate2_type, format::bfyx, input2_size });
        set_values(input2_mem, input2_data_bfyx);

        std::vector<output_type> out_data(p.b_out_num * p.f_out_num * p.m_size * p.n_size);

        for (size_t b = 0; b < p.b_out_num; ++b) {
            for (size_t f = 0; f < p.f_out_num; ++f) {
                for (size_t y = 0; y < p.m_size; ++y) {
                    for (size_t x = 0; x < p.n_size; ++x) {
                        size_t input2_data_index = getGemmIndex(x, y, f, b, x2_size, y2_size, p.f2_num, p.b2_num, x2_pitch, y2_pitch, f2_pitch, b2_pitch);
                        size_t out_data_index = getGemmIndex(x, y, f, b, x_out_size, y_out_size, p.f_out_num, p.b_out_num,
                                                             x_out_pitch, y_out_pitch, f_out_pitch, b_out_pitch);
                        accumulator_type acc = 0;

                        for (size_t k = 0; k < p.k_size; ++k) {
                            size_t input0_data_index = getGemmIndex(k * (!p.transpose_input0) + y * p.transpose_input0, y * (!p.transpose_input0) +
                            k * p.transpose_input0, f, b, x0_size, y0_size, p.f0_num, p.b0_num, x0_pitch, y0_pitch, f0_pitch, b0_pitch);
                            size_t input1_data_index = getGemmIndex(x * (!p.transpose_input1) + k * p.transpose_input1, k * (!p.transpose_input1) +
                            x * p.transpose_input1, f, b, x1_size, y1_size, p.f1_num, p.b1_num, x1_pitch, y1_pitch, f1_pitch, b1_pitch);
                            acc += (accumulator_type)input0_data_bfyx[input0_data_index] * (accumulator_type)input1_data_bfyx[input1_data_index];
                        }

                        out_data[out_data_index] = (output_type)acc;
                        out_data[out_data_index] *= (output_type)p.alpha;
                        if (p.beta)
                            out_data[out_data_index] += (output_type)p.beta * (output_type)input2_data_bfyx[input2_data_index];
                    }
                }
            }
        }

        topology topology;
        topology.add(input_layout("input0", input0_mem->get_layout()));
        topology.add(input_layout("input1", input1_mem->get_layout()));
        if (p.beta != 0) {
            topology.add(input_layout("input2", input2_mem->get_layout()));
            topology.add(gemm("gemm_bfyx", { "input0", "input1", "input2" }, p.output_type, p.transpose_input0, p.transpose_input1, p.alpha, p.beta));
        } else {
            topology.add(gemm("gemm_bfyx", { "input0", "input1" }, p.output_type, p.transpose_input0, p.transpose_input1, p.alpha, p.beta));
        }
        topology.add(reorder("reorder_bfyx", "gemm_bfyx", format::bfyx, data_types::f32));

        build_options options;
#ifdef ENABLE_ONEDNN_FOR_GPU
        implementation_desc gemm_impl = { format::bfyx, "", impl_types::onednn };
#else
        implementation_desc gemm_impl = { format::bfyx, p.kernel_name };
#endif
        options.set_option(build_option::force_implementations({ {"gemm_bfyx", gemm_impl} }));

        network network(engine, topology, options);
        network.set_input_data("input0", input0_mem);
        network.set_input_data("input1", input1_mem);
        if (p.beta != 0) {
            network.set_input_data("input2", input2_mem);
        }
        auto outputs = network.execute();
        auto output = outputs.at("reorder_bfyx").get_memory();
        cldnn::mem_lock<float> output_ptr(output, get_test_stream());

        const float threshold_int8 = 1.f;
        const float threshold_fp16 = 1e-1;
        const float threshold_fp32 = 3e-4;

        EXPECT_EQ(output_ptr.size(), (size_t)(p.b_out_num * p.f_out_num * p.m_size * p.n_size));
        if (sizeof(input0_type) == 1) {
            for (size_t i = 0; i < out_data.size(); ++i) {
                EXPECT_NEAR(float(output_ptr[i]), float(out_data[i]), threshold_int8) << "index = " << i;
            }
        } else if (sizeof(input0_type) == 2) {
            for (size_t i = 0; i < out_data.size(); ++i) {
                EXPECT_NEAR(float(output_ptr[i]), float(out_data[i]), threshold_fp16) << "index = " << i;
            }
        } else {
            for (size_t i = 0; i < out_data.size(); ++i) {
                EXPECT_NEAR(float(output_ptr[i]), float(out_data[i]), threshold_fp32) << "index = " << i;
            }
        }
    }
};

#ifdef ENABLE_ONEDNN_FOR_GPU

class gemm_int8_simple_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, int8_t, int8_t, float, float, int32_t> {};
TEST_P(gemm_int8_simple_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_int8_simple_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_INT8_ONEDNN_1, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_ONEDNN_2, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_ONEDNN_3, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_ONEDNN_4, "" },
}));

class gemm_uint8_simple_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, uint8_t, int8_t, float, float, int32_t> {};
TEST_P(gemm_uint8_simple_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_uint8_simple_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_UINT8_ONEDNN_1, "" },
    gemm_base_test_params{ CASE_GEMM_UINT8_ONEDNN_2, "" },
    gemm_base_test_params{ CASE_GEMM_UINT8_ONEDNN_3, "" },
    gemm_base_test_params{ CASE_GEMM_UINT8_ONEDNN_4, "" },
}));

class gemm_fp16_simple_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, FLOAT16, FLOAT16, FLOAT16, FLOAT16, FLOAT16> {};
TEST_P(gemm_fp16_simple_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp16_simple_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP16_ONEDNN_1, "" },
    gemm_base_test_params{ CASE_GEMM_FP16_ONEDNN_2, "" },
    gemm_base_test_params{ CASE_GEMM_FP16_ONEDNN_3, "" },
    gemm_base_test_params{ CASE_GEMM_FP16_ONEDNN_4, "" },
}));

class gemm_fp32_simple_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, float, float, float, float, float> {};
TEST_P(gemm_fp32_simple_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp32_simple_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP32_ONEDNN_1, "" },
    gemm_base_test_params{ CASE_GEMM_FP32_ONEDNN_2, "" },
    gemm_base_test_params{ CASE_GEMM_FP32_ONEDNN_3, "" },
    gemm_base_test_params{ CASE_GEMM_FP32_ONEDNN_4, "" },
}));

class gemm_int8_transposition_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, int8_t, int8_t, float, float, int32_t> {};
TEST_P(gemm_int8_transposition_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_int8_transposition_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_INT8_NN_TRANSPOSITION_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_NT_TRANSPOSITION_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_TN_TRANSPOSITION_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_TT_TRANSPOSITION_ONEDNN, "" },

    gemm_base_test_params{ CASE_GEMM_INT8_NN_TRANSPOSITION_LEFTOVERS_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_NT_TRANSPOSITION_LEFTOVERS_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_TN_TRANSPOSITION_LEFTOVERS_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_TT_TRANSPOSITION_LEFTOVERS_ONEDNN, "" },
}));

class gemm_uint8_transposition_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, uint8_t, int8_t, float, float, int32_t> {};
TEST_P(gemm_uint8_transposition_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_uint8_transposition_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_UINT8_NN_TRANSPOSITION_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_UINT8_NT_TRANSPOSITION_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_UINT8_TN_TRANSPOSITION_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_UINT8_TT_TRANSPOSITION_ONEDNN, "" },

    gemm_base_test_params{ CASE_GEMM_UINT8_NN_TRANSPOSITION_LEFTOVERS_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_UINT8_NT_TRANSPOSITION_LEFTOVERS_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_UINT8_TN_TRANSPOSITION_LEFTOVERS_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_UINT8_TT_TRANSPOSITION_LEFTOVERS_ONEDNN, "" },
}));

class gemm_fp16_transposition_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, FLOAT16, FLOAT16, FLOAT16, FLOAT16, FLOAT16> {};
TEST_P(gemm_fp16_transposition_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp16_transposition_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP16_NN_TRANSPOSITION_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_FP16_NT_TRANSPOSITION_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_FP16_TN_TRANSPOSITION_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_FP16_TT_TRANSPOSITION_ONEDNN, "" },
}));

class gemm_fp32_transposition_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, float, float, float, float, float> {};
TEST_P(gemm_fp32_transposition_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp32_transposition_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP32_NN_TRANSPOSITION_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_FP32_NT_TRANSPOSITION_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_FP32_TN_TRANSPOSITION_ONEDNN, "" },
    gemm_base_test_params{ CASE_GEMM_FP32_TT_TRANSPOSITION_ONEDNN, "" },
}));

class gemm_int8_broadcasting_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, int8_t, int8_t, float, float, int32_t> {};
TEST_P(gemm_int8_broadcasting_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_int8_broadcasting_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_INT8_BROADCASTING_ONEDNN_1, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_BROADCASTING_ONEDNN_2, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_BROADCASTING_ONEDNN_3, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_BROADCASTING_ONEDNN_4, "" },
}));

class gemm_fp16_broadcasting_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, FLOAT16, FLOAT16, FLOAT16, FLOAT16, FLOAT16> {};
TEST_P(gemm_fp16_broadcasting_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp16_broadcasting_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP16_BROADCASTING_ONEDNN_1, "" },
    gemm_base_test_params{ CASE_GEMM_FP16_BROADCASTING_ONEDNN_2, "" },
    gemm_base_test_params{ CASE_GEMM_FP16_BROADCASTING_ONEDNN_3, "" },
    gemm_base_test_params{ CASE_GEMM_FP16_BROADCASTING_ONEDNN_4, "" },
}));

class gemm_fp32_broadcasting_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, float, float, float, float, int32_t> {};
TEST_P(gemm_fp32_broadcasting_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp32_broadcasting_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP32_BROADCASTING_ONEDNN_1, "" },
    gemm_base_test_params{ CASE_GEMM_FP32_BROADCASTING_ONEDNN_2, "" },
    gemm_base_test_params{ CASE_GEMM_FP32_BROADCASTING_ONEDNN_3, "" },
    gemm_base_test_params{ CASE_GEMM_FP32_BROADCASTING_ONEDNN_4, "" },
}));

class gemm_int8_combo_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, int8_t, int8_t, float, float, int32_t> {};
TEST_P(gemm_int8_combo_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_int8_combo_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_INT8_COMBO_ONEDNN_1, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_COMBO_ONEDNN_2, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_COMBO_ONEDNN_3, "" },
    gemm_base_test_params{ CASE_GEMM_INT8_COMBO_ONEDNN_4, "" },
}));

class gemm_uint8_combo_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, uint8_t, int8_t, float, float, int32_t> {};
TEST_P(gemm_uint8_combo_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_uint8_combo_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_UINT8_COMBO_ONEDNN_1, "" },
    gemm_base_test_params{ CASE_GEMM_UINT8_COMBO_ONEDNN_2, "" },
    gemm_base_test_params{ CASE_GEMM_UINT8_COMBO_ONEDNN_3, "" },
    gemm_base_test_params{ CASE_GEMM_UINT8_COMBO_ONEDNN_4, "" },
}));

class gemm_fp16_combo_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, FLOAT16, FLOAT16, FLOAT16, FLOAT16, FLOAT16> {};
TEST_P(gemm_fp16_combo_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp16_combo_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP16_COMBO_ONEDNN_1, "" },
    gemm_base_test_params{ CASE_GEMM_FP16_COMBO_ONEDNN_2, "" },
    gemm_base_test_params{ CASE_GEMM_FP16_COMBO_ONEDNN_3, "" },
    gemm_base_test_params{ CASE_GEMM_FP16_COMBO_ONEDNN_4, "" },
}));

class gemm_fp32_combo_tests_onednn : public ::GemmBaseTest<gemm_base_test_params, float, float, float, float, float> {};
TEST_P(gemm_fp32_combo_tests_onednn, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp32_combo_tests_onednn, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP32_COMBO_ONEDNN_1, "" },
    gemm_base_test_params{ CASE_GEMM_FP32_COMBO_ONEDNN_2, "" },
    gemm_base_test_params{ CASE_GEMM_FP32_COMBO_ONEDNN_3, "" },
    gemm_base_test_params{ CASE_GEMM_FP32_COMBO_ONEDNN_4, "" },
}));

#else // ENABLE_ONEDNN_FOR_GPU

class gemm_int8_transposition_tests : public ::GemmBaseTest<gemm_base_test_params, int8_t, uint8_t, float, float, int32_t> {};
TEST_P(gemm_int8_transposition_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_int8_transposition_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_INT8_NN_TRANSPOSITION, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_NT_TRANSPOSITION, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_TN_TRANSPOSITION, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_TT_TRANSPOSITION, "gemm_mmad_int8" },
}));

class gemm_int8_broadcast_tests : public ::GemmBaseTest<gemm_base_test_params, int8_t, uint8_t, float, float, int32_t> {};
TEST_P(gemm_int8_broadcast_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_int8_broadcast_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_INT8_BROADCAST_1, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_BROADCAST_2, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_BROADCAST_3, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_BROADCAST_4, "gemm_mmad_int8" },
}));

class gemm_int8_leftovers_tests : public ::GemmBaseTest<gemm_base_test_params, int8_t, uint8_t, float, float, int32_t> {};
TEST_P(gemm_int8_leftovers_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_int8_leftovers_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_INT8_LEFTOVERS_1, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_LEFTOVERS_2, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_LEFTOVERS_3, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_LEFTOVERS_4, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_LEFTOVERS_5, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_LEFTOVERS_6, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_LEFTOVERS_7, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_LEFTOVERS_8, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_LEFTOVERS_9, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_LEFTOVERS_10, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_LEFTOVERS_11, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_LEFTOVERS_12, "gemm_mmad_int8" },
}));

class gemm_int8_combo_tests : public ::GemmBaseTest<gemm_base_test_params, int8_t, uint8_t, float, float, int32_t> {};
TEST_P(gemm_int8_combo_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_int8_combo_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_INT8_COMBO_1, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_COMBO_2, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_COMBO_3, "gemm_mmad_int8" },
    gemm_base_test_params{ CASE_GEMM_INT8_COMBO_4, "gemm_mmad_int8" },
}));

class gemm_int8_slm_combo_tests : public ::GemmBaseTest<gemm_base_test_params, int8_t, uint8_t, float, float, int32_t> {};
TEST_P(gemm_int8_slm_combo_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_int8_slm_combo_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_INT8_SLM_COMBO_1, "gemm_mmad_int8_slm" },
    gemm_base_test_params{ CASE_GEMM_INT8_SLM_COMBO_2, "gemm_mmad_int8_slm" },
    gemm_base_test_params{ CASE_GEMM_INT8_SLM_COMBO_3, "gemm_mmad_int8_slm" },
    gemm_base_test_params{ CASE_GEMM_INT8_SLM_COMBO_4, "gemm_mmad_int8_slm" },
}));

class gemm_fp32_tiled_nn_tests : public ::GemmBaseTest<gemm_base_test_params, float, float, float, float, float> {};
TEST_P(gemm_fp32_tiled_nn_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp32_tiled_nn_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_NN_1, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_NN_2, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_NN_3, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_NN_4, "gemm_tiled_opt" },
}));

class gemm_fp32_tiled_nt_tests : public ::GemmBaseTest<gemm_base_test_params, float, float, float, float, float> {};
TEST_P(gemm_fp32_tiled_nt_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp32_tiled_nt_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_NT_1, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_NT_2, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_NT_3, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_NT_4, "gemm_tiled_opt" },
}));

class gemm_fp32_tiled_tn_tests : public ::GemmBaseTest<gemm_base_test_params, float, float, float, float, float> {};
TEST_P(gemm_fp32_tiled_tn_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp32_tiled_tn_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_TN_1, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_TN_2, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_TN_3, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_TN_4, "gemm_tiled_opt" },
}));

class gemm_fp32_tiled_tt_tests : public ::GemmBaseTest<gemm_base_test_params, float, float, float, float, float> {};
TEST_P(gemm_fp32_tiled_tt_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp32_tiled_tt_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_TT_1, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_TT_2, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_TT_3, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_TT_4, "gemm_tiled_opt" },
}));

class gemm_fp32_tiled_nn_broadcast_tests : public ::GemmBaseTest<gemm_base_test_params, float, float, float, float, float> {};
TEST_P(gemm_fp32_tiled_nn_broadcast_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp32_tiled_nn_broadcast_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_NN_BROADCAST_1, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_NN_BROADCAST_2, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_NN_BROADCAST_3, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP32_TILED_NN_BROADCAST_4, "gemm_tiled_opt" },
}));

class gemm_fp16_tiled_nn_tests : public ::GemmBaseTest<gemm_base_test_params, FLOAT16, FLOAT16, FLOAT16, FLOAT16, FLOAT16> {};
TEST_P(gemm_fp16_tiled_nn_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp16_tiled_nn_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_NN_1, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_NN_2, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_NN_3, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_NN_4, "gemm_tiled_opt" },
}));

class gemm_fp16_tiled_nt_tests : public ::GemmBaseTest<gemm_base_test_params, FLOAT16, FLOAT16, FLOAT16, FLOAT16, FLOAT16> {};
TEST_P(gemm_fp16_tiled_nt_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp16_tiled_nt_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_NT_1, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_NT_2, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_NT_3, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_NT_4, "gemm_tiled_opt" },
}));

class gemm_fp16_tiled_tn_tests : public ::GemmBaseTest<gemm_base_test_params, FLOAT16, FLOAT16, FLOAT16, FLOAT16, FLOAT16> {};
TEST_P(gemm_fp16_tiled_tn_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp16_tiled_tn_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_TN_1, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_TN_2, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_TN_3, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_TN_4, "gemm_tiled_opt" },
}));

class gemm_fp16_tiled_tt_tests : public ::GemmBaseTest<gemm_base_test_params, FLOAT16, FLOAT16, FLOAT16, FLOAT16, FLOAT16> {};
TEST_P(gemm_fp16_tiled_tt_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp16_tiled_tt_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_TT_1, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_TT_2, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_TT_3, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_TT_4, "gemm_tiled_opt" },
}));

class gemm_fp16_tiled_nn_broadcast_tests : public ::GemmBaseTest<gemm_base_test_params, FLOAT16, FLOAT16, FLOAT16, FLOAT16, FLOAT16> {};
TEST_P(gemm_fp16_tiled_nn_broadcast_tests, basic) { auto p = GetParam(); execute(p); }

INSTANTIATE_TEST_SUITE_P(gemm_gpu, gemm_fp16_tiled_nn_broadcast_tests, ::testing::ValuesIn(std::vector <gemm_base_test_params> {
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_NN_BROADCAST_1, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_NN_BROADCAST_2, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_NN_BROADCAST_3, "gemm_tiled_opt" },
    gemm_base_test_params{ CASE_GEMM_FP16_TILED_NN_BROADCAST_4, "gemm_tiled_opt" },
}));

#endif // ENABLE_ONEDNN_FOR_GPU
