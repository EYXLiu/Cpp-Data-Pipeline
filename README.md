# Latency Focused Data Pipeline
Tech stack: C++, Kraken Websocket  
Previously was a networking runtime, being repurposed to a C++ data pipeline, focusing on latency (I lost track of what I was doing and wanted a clear goal)

# Inital baseline 
commit 8cac493da0753205929f9d66786bf87fa9a778fd  
nholmann json, stod, 1000 samples  
|Step      |Median Percentile|90th Percentile  |99th Percentile  |
|----------|-----------------|-----------------|-----------------|
| PARSE    | p50=18.959us    | p90=36.083us    | p99=60.791us    |
| DISPATCH | p50=1.875us     | p90=8.625us     | p99=58.625us    |
| PROCESS  | p50=0.042us     | p90=0.042us     | p99=0.083us     |
| TOTAL    | p50=21.042us    | p90=46.458us    | p99=128.916us   |
 - High parse times (likely due to JSON and stod) 
 - ~~Wildly varying dispatch times (possibly system error?)~~
 - EDIT: Was because I was calculating stod as part of dispatch, will retest another time  
 - Process times look ok 


# REGEX
commit 14798d2b860cdf4a2fb91d3532168a126b1b1dcc  
regex, stod, 1000 samples  
|Step      |Median Percentile|90th Percentile  |99th Percentile  |
|----------|-----------------|-----------------|-----------------|
| PARSE    | p50=130.333us   | p90=195.75us    | p99=554.584us   |
| DISPATCH | p50=3.208us     | p90=5.083us     | p99=12.584us    |
| PROCESS  | p50=0.042us     | p90=0.042us     | p99=0.042us     |
| TOTAL    | p50=133.667us   | p90=202.208us   | p99=631.625us   |
 - Significantly worse than JSON (I thought it would be faster just cuz it's string parsing)  
 - ~~Dispatch times similar to above, probably just cpu error~~
 - EDIT: Was because I was calculating stod as part of dispatch, refer to simdjson and below for proper process times
 - Process times also similar to above  
 - Will try basic basic string parsing next to see if it's faster

# simdjson  
commit af1ce97b779d460178dd120eb34463e0f4c79cf5  
simdjson, stod, 1000 samples  
|Step      |Median Percentile|90th Percentile  |99th Percentile  |
|----------|-----------------|-----------------|-----------------|
| PARSE    | p50=6.25us      | p90=15.292us    | p99=57.209us    |
| DISPATCH | p50=0.042us     | p90=0.291us     | p99=0.459us     |
| PROCESS  | p50=0.041us     | p90=0.042us     | p99=0.042us     |
| TOTAL    | p50=6.291us     | p90=15.459us    | p99=57.458us    |
- Significantly better than nohlman json, roughly threefold faster
- simdjson is optimized for fast parsing
- Dispatch and process times are very quick, as expected from trivial calculations
- Will test rapidjson and then custom parsing

# rapidjson
commit 643bee7f702f65b591d067e40e6eb7572a81fddf  
rapidjson, stod, 1000 samples  
|Step      |Median Percentile|90th Percentile  |99th Percentile  |
|----------|-----------------|-----------------|-----------------|
| PARSE    | p50=14.5us      | p90=43.666us    | p99=225.834us   |
| DISPATCH | p50=0.042us     | p90=0.708us     | p99=1.291us     |
| PROCESS  | p50=0.042us     | p90=0.042us     | p99=0.084us     |
| TOTAL    | p50=14.666us    | p90=44.5us      | p99=226us       |
- Slightly worse than simdjson, still better than nohlman json though  
- warning: rapidjson is depricated (uses std::iterator which has depricated since c++17)  
- still usable, but will generate warnings; currently on c++20, make sure the current c++ version has only depricated and not fully removed std::iterator  
- Dispatch and process times same as above  
- Will test custom parsing now  
