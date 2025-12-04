/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#include <nanobench.h>
#include <snitch_all.hpp>

#include <patum.h>

#include <cstdint>
#include <algorithm>
#include <fstream>
#include <random>
#include <vector>

//==================================================================================================

namespace nanobench = ankerl::nanobench;

namespace {

auto generate_data(int min, int max, std::size_t size)
{
    std::random_device dev;
    std::uniform_int_distribution<int> dist(min, max);
    nanobench::Rng gen(dev());

    std::vector<int> data(size);

    std::generate(data.begin(), data.end(), [&] { return dist(gen); });

    return data;
}

auto generate_tuple_data(int min, int max, std::size_t size)
{
    std::random_device dev;
    std::uniform_int_distribution<int> dist(min, max);
    nanobench::Rng gen(dev());

    std::vector<std::tuple<int, int, int>> data(size);

    std::generate(data.begin(), data.end(), [&]
    {
        return std::make_tuple(dist(gen), dist(gen), dist(gen));
    });

    return data;
}

void generate_output(const std::string& type_name, const char* mustache_template, const nanobench::Bench& bench)
{
    std::string title = bench.title();
    std::ranges::transform(title, title.begin(), [](unsigned char c)
    {
        return std::isspace(c) ? '_' : std::tolower(c);
    });

    std::ofstream template_out("bench.template." + title + "." + type_name);
    template_out << mustache_template;

    std::ofstream render_out("bench.render." + title + "." + type_name);
    nanobench::render(mustache_template, bench, render_out);
}

inline static constexpr char const* html_boxplot = R"DELIM(<html>

<head>
    <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
</head>

<body>
    <div id="myDiv"></div>
    <script>
        var data = [
            {{#result}}{
                name: '{{name}}',
                y: [{{#measurement}}{{elapsed}}{{^-last}}, {{/last}}{{/measurement}}],
            },
            {{/result}}
        ];

        data = data.map(a => Object.assign(a, { boxpoints: 'all', pointpos: 0, type: 'box' }));

        var template = {"layout":{"autotypenumbers":"strict","colorway":["#636efa","#EF553B","#00cc96","#ab63fa","#FFA15A","#19d3f3","#FF6692","#B6E880","#FF97FF","#FECB52"],"font":{"color":"#f2f5fa"},"hovermode":"closest","hoverlabel":{"align":"left"},"paper_bgcolor":"rgb(17,17,17)","plot_bgcolor":"rgb(17,17,17)","polar":{"bgcolor":"rgb(17,17,17)","angularaxis":{"gridcolor":"#506784","linecolor":"#506784","ticks":""},"radialaxis":{"gridcolor":"#506784","linecolor":"#506784","ticks":""}},"ternary":{"bgcolor":"rgb(17,17,17)","aaxis":{"gridcolor":"#506784","linecolor":"#506784","ticks":""},"baxis":{"gridcolor":"#506784","linecolor":"#506784","ticks":""},"caxis":{"gridcolor":"#506784","linecolor":"#506784","ticks":""}},"coloraxis":{"colorbar":{"outlinewidth":0,"ticks":""}},"colorscale":{"sequential":[[0.0,"#0d0887"],[0.1111111111111111,"#46039f"],[0.2222222222222222,"#7201a8"],[0.3333333333333333,"#9c179e"],[0.4444444444444444,"#bd3786"],[0.5555555555555556,"#d8576b"],[0.6666666666666666,"#ed7953"],[0.7777777777777778,"#fb9f3a"],[0.8888888888888888,"#fdca26"],[1.0,"#f0f921"]],"sequentialminus":[[0.0,"#0d0887"],[0.1111111111111111,"#46039f"],[0.2222222222222222,"#7201a8"],[0.3333333333333333,"#9c179e"],[0.4444444444444444,"#bd3786"],[0.5555555555555556,"#d8576b"],[0.6666666666666666,"#ed7953"],[0.7777777777777778,"#fb9f3a"],[0.8888888888888888,"#fdca26"],[1.0,"#f0f921"]],"diverging":[[0,"#8e0152"],[0.1,"#c51b7d"],[0.2,"#de77ae"],[0.3,"#f1b6da"],[0.4,"#fde0ef"],[0.5,"#f7f7f7"],[0.6,"#e6f5d0"],[0.7,"#b8e186"],[0.8,"#7fbc41"],[0.9,"#4d9221"],[1,"#276419"]]},"xaxis":{"gridcolor":"#283442","linecolor":"#506784","ticks":"","title":{"standoff":15},"zerolinecolor":"#283442","automargin":true,"zerolinewidth":2},"yaxis":{"gridcolor":"#283442","linecolor":"#506784","ticks":"","title":{"standoff":15},"zerolinecolor":"#283442","automargin":true,"zerolinewidth":2},"scene":{"xaxis":{"backgroundcolor":"rgb(17,17,17)","gridcolor":"#506784","linecolor":"#506784","showbackground":true,"ticks":"","zerolinecolor":"#C8D4E3","gridwidth":2},"yaxis":{"backgroundcolor":"rgb(17,17,17)","gridcolor":"#506784","linecolor":"#506784","showbackground":true,"ticks":"","zerolinecolor":"#C8D4E3","gridwidth":2},"zaxis":{"backgroundcolor":"rgb(17,17,17)","gridcolor":"#506784","linecolor":"#506784","showbackground":true,"ticks":"","zerolinecolor":"#C8D4E3","gridwidth":2}},"shapedefaults":{"line":{"color":"#f2f5fa"}},"annotationdefaults":{"arrowcolor":"#f2f5fa","arrowhead":0,"arrowwidth":1},"geo":{"bgcolor":"rgb(17,17,17)","landcolor":"rgb(17,17,17)","subunitcolor":"#506784","showland":true,"showlakes":true,"lakecolor":"rgb(17,17,17)"},"title":{"x":0.05},"updatemenudefaults":{"bgcolor":"#506784","borderwidth":0},"sliderdefaults":{"bgcolor":"#C8D4E3","borderwidth":1,"bordercolor":"rgb(17,17,17)","tickwidth":0},"mapbox":{"style":"dark"}},"data":{"histogram2dcontour":[{"type":"histogram2dcontour","colorbar":{"outlinewidth":0,"ticks":""},"colorscale":[[0.0,"#0d0887"],[0.1111111111111111,"#46039f"],[0.2222222222222222,"#7201a8"],[0.3333333333333333,"#9c179e"],[0.4444444444444444,"#bd3786"],[0.5555555555555556,"#d8576b"],[0.6666666666666666,"#ed7953"],[0.7777777777777778,"#fb9f3a"],[0.8888888888888888,"#fdca26"],[1.0,"#f0f921"]]}],"choropleth":[{"type":"choropleth","colorbar":{"outlinewidth":0,"ticks":""}}],"histogram2d":[{"type":"histogram2d","colorbar":{"outlinewidth":0,"ticks":""},"colorscale":[[0.0,"#0d0887"],[0.1111111111111111,"#46039f"],[0.2222222222222222,"#7201a8"],[0.3333333333333333,"#9c179e"],[0.4444444444444444,"#bd3786"],[0.5555555555555556,"#d8576b"],[0.6666666666666666,"#ed7953"],[0.7777777777777778,"#fb9f3a"],[0.8888888888888888,"#fdca26"],[1.0,"#f0f921"]]}],"heatmap":[{"type":"heatmap","colorbar":{"outlinewidth":0,"ticks":""},"colorscale":[[0.0,"#0d0887"],[0.1111111111111111,"#46039f"],[0.2222222222222222,"#7201a8"],[0.3333333333333333,"#9c179e"],[0.4444444444444444,"#bd3786"],[0.5555555555555556,"#d8576b"],[0.6666666666666666,"#ed7953"],[0.7777777777777778,"#fb9f3a"],[0.8888888888888888,"#fdca26"],[1.0,"#f0f921"]]}],"heatmapgl":[{"type":"heatmapgl","colorbar":{"outlinewidth":0,"ticks":""},"colorscale":[[0.0,"#0d0887"],[0.1111111111111111,"#46039f"],[0.2222222222222222,"#7201a8"],[0.3333333333333333,"#9c179e"],[0.4444444444444444,"#bd3786"],[0.5555555555555556,"#d8576b"],[0.6666666666666666,"#ed7953"],[0.7777777777777778,"#fb9f3a"],[0.8888888888888888,"#fdca26"],[1.0,"#f0f921"]]}],"contourcarpet":[{"type":"contourcarpet","colorbar":{"outlinewidth":0,"ticks":""}}],"contour":[{"type":"contour","colorbar":{"outlinewidth":0,"ticks":""},"colorscale":[[0.0,"#0d0887"],[0.1111111111111111,"#46039f"],[0.2222222222222222,"#7201a8"],[0.3333333333333333,"#9c179e"],[0.4444444444444444,"#bd3786"],[0.5555555555555556,"#d8576b"],[0.6666666666666666,"#ed7953"],[0.7777777777777778,"#fb9f3a"],[0.8888888888888888,"#fdca26"],[1.0,"#f0f921"]]}],"surface":[{"type":"surface","colorbar":{"outlinewidth":0,"ticks":""},"colorscale":[[0.0,"#0d0887"],[0.1111111111111111,"#46039f"],[0.2222222222222222,"#7201a8"],[0.3333333333333333,"#9c179e"],[0.4444444444444444,"#bd3786"],[0.5555555555555556,"#d8576b"],[0.6666666666666666,"#ed7953"],[0.7777777777777778,"#fb9f3a"],[0.8888888888888888,"#fdca26"],[1.0,"#f0f921"]]}],"mesh3d":[{"type":"mesh3d","colorbar":{"outlinewidth":0,"ticks":""}}],"scatter":[{"marker":{"line":{"color":"#283442"}},"type":"scatter"}],"parcoords":[{"type":"parcoords","line":{"colorbar":{"outlinewidth":0,"ticks":""}}}],"scatterpolargl":[{"type":"scatterpolargl","marker":{"colorbar":{"outlinewidth":0,"ticks":""}}}],"bar":[{"error_x":{"color":"#f2f5fa"},"error_y":{"color":"#f2f5fa"},"marker":{"line":{"color":"rgb(17,17,17)","width":0.5},"pattern":{"fillmode":"overlay","size":10,"solidity":0.2}},"type":"bar"}],"scattergeo":[{"type":"scattergeo","marker":{"colorbar":{"outlinewidth":0,"ticks":""}}}],"scatterpolar":[{"type":"scatterpolar","marker":{"colorbar":{"outlinewidth":0,"ticks":""}}}],"histogram":[{"marker":{"pattern":{"fillmode":"overlay","size":10,"solidity":0.2}},"type":"histogram"}],"scattergl":[{"marker":{"line":{"color":"#283442"}},"type":"scattergl"}],"scatter3d":[{"type":"scatter3d","line":{"colorbar":{"outlinewidth":0,"ticks":""}},"marker":{"colorbar":{"outlinewidth":0,"ticks":""}}}],"scattermapbox":[{"type":"scattermapbox","marker":{"colorbar":{"outlinewidth":0,"ticks":""}}}],"scatterternary":[{"type":"scatterternary","marker":{"colorbar":{"outlinewidth":0,"ticks":""}}}],"scattercarpet":[{"type":"scattercarpet","marker":{"colorbar":{"outlinewidth":0,"ticks":""}}}],"carpet":[{"aaxis":{"endlinecolor":"#A2B1C6","gridcolor":"#506784","linecolor":"#506784","minorgridcolor":"#506784","startlinecolor":"#A2B1C6"},"baxis":{"endlinecolor":"#A2B1C6","gridcolor":"#506784","linecolor":"#506784","minorgridcolor":"#506784","startlinecolor":"#A2B1C6"},"type":"carpet"}],"table":[{"cells":{"fill":{"color":"#506784"},"line":{"color":"rgb(17,17,17)"}},"header":{"fill":{"color":"#2a3f5f"},"line":{"color":"rgb(17,17,17)"}},"type":"table"}],"barpolar":[{"marker":{"line":{"color":"rgb(17,17,17)","width":0.5},"pattern":{"fillmode":"overlay","size":10,"solidity":0.2}},"type":"barpolar"}],"pie":[{"automargin":true,"type":"pie"}]}}

        var layout = {
            title: { text: '{{title}}' },
            showlegend: false,
            template: template,
            yaxis: { title: 'time per unit', rangemode: 'tozero', autorange: true }
        };

        Plotly.newPlot('myDiv', data, layout, { responsive: true });
    </script>
</body>

</html>)DELIM";

} // namespace

//==================================================================================================

TEST_CASE("matching_performance", "[base]")
{
    auto b = nanobench::Bench()
        .title("Simple Switch")
        .warmup(100)
        .minEpochIterations(8000000)
        .performanceCounters(true)
        .relative(true);

    const auto data = generate_data(0, 8, 100000);
    std::size_t counter = 0;

    counter = 0;
    b.run("patum", [&]
    {
        using namespace ptm;

        const auto x = data[counter];
        counter = (++counter) % data.size();

        auto result = match(x)
        (
            pattern(1) = 1,
            pattern(2) = 20,
            pattern(3) = 300,
            pattern(4) = 4000
        ).value_or(0);

        nanobench::doNotOptimizeAway(result);
    });

    counter = 0;
    b.run("if", [&]
    {
        const auto x = data[counter];
        counter = (++counter) % data.size();

        int result;

        if (x == 1)
            result = 1;
        else if (x == 2)
            result = 20;
        else if (x == 3)
            result = 300;
        else if (x == 4)
            result = 4000;
        else
            result = 0;

        nanobench::doNotOptimizeAway(result);
    });

    counter = 0;
    b.run("switch", [&]
    {
        const auto x = data[counter];
        counter = (++counter) % data.size();

        int result;

        switch(x)
        {
            case 1: result = 1; break;
            case 2: result = 20; break;
            case 3: result = 300; break;
            case 4: result = 4000; break;
            default: result = 0; break;
        }

        nanobench::doNotOptimizeAway(result);
    });

    generate_output("html", html_boxplot, b);
}

//==================================================================================================

TEST_CASE("tuple_destructuring", "[tuple]")
{
    auto b = nanobench::Bench()
        .title("Tuple Destructuring")
        .warmup(100)
        .minEpochIterations(8000000)
        .performanceCounters(true)
        .relative(true);

    const auto data = generate_tuple_data(0, 8, 100000);
    std::size_t counter = 0;

    counter = 0;
    b.run("patum", [&]
    {
        using namespace ptm;

        const auto& x = data[counter];
        counter = (++counter) % data.size();

        auto result = match(x)
        (
            pattern(ds(1, _, 1)) = 1,
            pattern(ds(2, _, 2)) = 20,
            pattern(ds(3, _, 3)) = 300,
            pattern(ds(4, _, 4)) = 4000
        ).value_or(0);

        nanobench::doNotOptimizeAway(result);
    });

    counter = 0;
    b.run("if", [&]
    {
        const auto& x = data[counter];
        counter = (++counter) % data.size();

        int result;

        int a = std::get<0>(x);
        int b = std::get<2>(x);

        if (a == 1 && b == 1)
            result = 1;
        else if (a == 2 && b == 2)
            result = 20;
        else if (a == 3 && b == 3)
            result = 300;
        else if (a == 4 && b == 4)
            result = 4000;
        else
            result = 0;

        nanobench::doNotOptimizeAway(result);
    });

    counter = 0;
    b.run("switch", [&]
    {
        const auto& x = data[counter];
        counter = (++counter) % data.size();

        int result;

        int a = std::get<0>(x);
        int b = std::get<2>(x);

        switch (a)
        {
            case 1:
                switch (b)
                {
                    case 1: result = 1; break;
                    default: result = 0; break;
                }
                break;

            case 2:
                switch (b)
                {
                    case 2: result = 20; break;
                    default: result = 0; break;
                }
                break;

            case 3:
                switch (b)
                {
                    case 3: result = 300; break;
                    default: result = 0; break;
                }
                break;

            case 4:
                switch (b)
                {
                    case 4: result = 4000; break;
                    default: result = 0; break;
                }
                break;

            default: result = 0; break;
        }

        nanobench::doNotOptimizeAway(result);
    });

    generate_output("html", html_boxplot, b);
}

//==================================================================================================

TEST_CASE("branchless", "[branching]")
{
    auto b = nanobench::Bench()
        .title("Branchless")
        .warmup(100)
        .minEpochIterations(8000000)
        .performanceCounters(true)
        .relative(true);

    const auto data = generate_data(0, 8, 100000);
    std::size_t counter = 0;

    counter = 0;
    b.run("patum", [&]
    {
        using namespace ptm;

        auto x = data[counter];
        counter = (++counter) % data.size();

        auto result = match(x)
        (
            pattern(_x < 1) = 1,
            pattern(_x < 2) = 20,
            pattern(_x < 3) = 300,
            pattern(_x < 4) = 4000
        ).value_or(0);

        nanobench::doNotOptimizeAway(result);
    });

    counter = 0;
    b.run("if", [&]
    {
        auto x = data[counter];
        counter = (++counter) % data.size();

        int result;

        if (x < 1)
            result = 1;
        else if (x < 2)
            result = 20;
        else if (x < 3)
            result = 300;
        else if (x < 4)
            result = 4000;
        else
            result = 0;

        nanobench::doNotOptimizeAway(result);
    });

    generate_output("html", html_boxplot, b);
}
