# Discussions

<input type="text" value="/Users/pdelgosha/Documents/Projects/Venkat/Codes/008_marked-graph-compression/report/report.md" id="myInput" readonly>
<button onclick="myFunction()">Copy Source Location</button>

<script>
function myFunction() {
  var copyText = document.getElementById("myInput");
  copyText.select();
  document.execCommand("copy");
}
</script>

## 2019-04-08 

Improved MP by avoiding doing things twice. Since the commit of 3/13 (8642e59), I have made the following modifications: 

1. Instead of using binary search tree for `message_dict`, a hash table is used to improved the speed. 
2. Avoid doing things multiple times: when a message is fond in the dictionary, instead of using the bracket operator to assign the corresponding message, one can use an iterator that is helpful in both checking whether the message exists in the dictionary and also this assignment. 

With these modifications, here is the result running time of the algorithm when applied to a Poisson random graph with 100,000 nodes, edge and vertex mark set size = 10, and average number of nodes connected to every vertex = 3 (average degree would be 6), h = 3, delta = 20:

<details>
<summary> Click here to open </summary>

```
|---Construct G () 2019-04-08 07:37:02 PM
 edges size 300041
 graph constructed
|---Encode () 2019-04-08 07:37:03 PM
|---|---Init compressed () 2019-04-08 07:37:03 PM
|---|---Extact edge types () 2019-04-08 07:37:03 PM
|---|---|---Extract messages () 2019-04-08 07:37:03 PM
|---|---|---|---graph_message::update_message init () 2019-04-08 07:37:03 PM
|---|---|---|---resizing messages () 2019-04-08 07:37:03 PM
|---|---|---|---initializing messages () 2019-04-08 07:37:03 PM
|---|---|---|---updating messages () 2019-04-08 07:37:03 PM
 total time to search in hash table: 0.769341
 total time to insert in hash table: 1.53727
 total time to modify vector m  0.799577
 total time to sort  0.0696137
|---|---|---|---* symmetrizing () 2019-04-08 07:37:09 PM
|---|---|---|---setting message_mark and is_star_message () 2019-04-08 07:37:09 PM
|---|---|---colored_graph::init init () 2019-04-08 07:37:09 PM
|---|---|---updating adj_list () 2019-04-08 07:37:10 PM
|---|---|---Find deg and ver_types () 2019-04-08 07:37:10 PM
|---|---Encode * vertices () 2019-04-08 07:37:11 PM
|---|---Encode * edges () 2019-04-08 07:37:14 PM
|---|---Encode vertex types () 2019-04-08 07:37:14 PM
|---|---Extract partition graphs () 2019-04-08 07:37:16 PM
|---|---Encode partition b graphs () 2019-04-08 07:37:19 PM
|---|---Encode partition graphs () 2019-04-08 07:37:21 PM
|---Decode () 2019-04-08 07:37:21 PM
|---|---Init () 2019-04-08 07:37:21 PM
|---|---Decode * vertices () 2019-04-08 07:37:21 PM
|---|---Decode * edges () 2019-04-08 07:37:24 PM
|---|---Decode vertex types () 2019-04-08 07:37:24 PM
|---|---Decode partition graphs () 2019-04-08 07:37:30 PM
|---|---Decode partition b graphs () 2019-04-08 07:37:30 PM
|---|---Construct decoded graph () 2019-04-08 07:37:34 PM
|---compare () 2019-04-08 07:37:35 PM
 successfully decoded the marked graph :D

|---Construct G (): 1.007256s [2.847803%]
|---Encode (): 18.077404s [51.110016%]
|---|---Init compressed (): 0.000786s [0.004350%]
|---|---Extact edge types (): 8.238246s [45.572063%]
|---|---|---Extract messages (): 6.366953s [77.285301%]
|---|---|---|---graph_message::update_message init (): 0.001501s [0.023580%]
|---|---|---|---resizing messages (): 0.177938s [2.794706%]
|---|---|---|---initializing messages (): 0.182283s [2.862949%]
|---|---|---|---updating messages (): 5.933976s [93.199608%]
|---|---|---|---* symmetrizing (): 0.035266s [0.553898%]
|---|---|---|---setting message_mark and is_star_message (): 0.035971s [0.564971%]
|---|---|---colored_graph::init init (): 0.224685s [2.727335%]
|---|---|---updating adj_list (): 0.393801s [4.780152%]
|---|---|---Find deg and ver_types (): 1.252763s [15.206671%]
|---|---Encode * vertices (): 2.368763s [13.103448%]
|---|---Encode * edges (): 0.000043s [0.000240%]
|---|---Encode vertex types (): 2.317106s [12.817693%]
|---|---Extract partition graphs (): 2.823593s [15.619463%]
|---|---Encode partition b graphs (): 2.143847s [11.859265%]
|---|---Encode partition graphs (): 0.182942s [1.011993%]
|---Decode (): 13.384299s [37.841259%]
|---|---Init (): 0.000024s [0.000179%]
|---|---Decode * vertices (): 2.418280s [18.068033%]
|---|---Decode * edges (): 0.000034s [0.000256%]
|---|---Decode vertex types (): 6.676574s [49.883629%]
|---|---Decode partition graphs (): 0.000054s [0.000400%]
|---|---Decode partition b graphs (): 3.761533s [28.104073%]
|---|---Construct decoded graph (): 0.527769s [3.943198%]
|---compare (): 2.900620s [8.200886%]
       35.38 real        33.98 user         0.73 sys
```

</details>


Here, extracting edge types takes 45% of the compression time, and updating messages takes 93% of extracting edge types. Updating messages takes 5.9s, among which 0.76s is for search in hash table, 1.5s is for insertion in hash table, and 0.8s for modifying vector m (the message vector). This totals to 3s, yet 2.9s remains which is not clear what happens. Maybe I should do a more search on this. I also thought of updating messages parallel since updating for different nodes are independent. So I spent some time last week to learn multi thread programming in C++. 


## 2019-04-09 8:53 PM

Realized that in updating messages, the variable `neighbor_messages` is defined in each iteration of the loop. I defined it outside the loop, reserved some space for it, and cleared it inside the loop. This made some improvement (around 0.1 - 0.2 s): 

<details>
<summary> before </summary>

```
---Construct G () 2019-04-09 05:03:30 PM
 edges size 299475
 graph constructed
|---Encode () 2019-04-09 05:03:31 PM
|---|---Init compressed () 2019-04-09 05:03:31 PM
|---|---Extact edge types () 2019-04-09 05:03:31 PM
|---|---|---Extract messages () 2019-04-09 05:03:31 PM
|---|---|---|---graph_message::update_message init () 2019-04-09 05:03:31 PM
|---|---|---|---resizing messages () 2019-04-09 05:03:31 PM
|---|---|---|---initializing messages () 2019-04-09 05:03:31 PM
|---|---|---|---updating messages () 2019-04-09 05:03:31 PM
 total time to search in hash table: 0.76763
 total time to insert in hash table: 1.55482
 total time to modify vector m  0.801063
 total time to sort  0.0744823
 total time to collect neighbor messages 1.05781
|---|---|---|---* symmetrizing () 2019-04-09 05:03:37 PM
|---|---|---|---setting message_mark and is_star_message () 2019-04-09 05:03:37 PM
|---|---|---colored_graph::init init () 2019-04-09 05:03:37 PM
|---|---|---updating adj_list () 2019-04-09 05:03:37 PM
|---|---|---Find deg and ver_types () 2019-04-09 05:03:38 PM
|---|---Encode * vertices () 2019-04-09 05:03:39 PM
|---|---Encode * edges () 2019-04-09 05:03:41 PM
|---|---Encode vertex types () 2019-04-09 05:03:41 PM
|---|---Extract partition graphs () 2019-04-09 05:03:44 PM
|---|---Encode partition b graphs () 2019-04-09 05:03:46 PM
|---|---Encode partition graphs () 2019-04-09 05:03:48 PM
|---Decode () 2019-04-09 05:03:49 PM
|---|---Init () 2019-04-09 05:03:49 PM
|---|---Decode * vertices () 2019-04-09 05:03:49 PM
|---|---Decode * edges () 2019-04-09 05:03:51 PM
|---|---Decode vertex types () 2019-04-09 05:03:51 PM
|---|---Decode partition graphs () 2019-04-09 05:03:57 PM
|---|---Decode partition b graphs () 2019-04-09 05:03:57 PM
|---|---Construct decoded graph () 2019-04-09 05:04:01 PM
|---compare () 2019-04-09 05:04:01 PM
 successfully decoded the marked graph :D

|---Construct G (): 0.945466s [2.729596%]
|---Encode (): 17.966131s [51.868896%]
|---|---Init compressed (): 0.000691s [0.003846%]
|---|---Extact edge types (): 8.346380s [46.456192%]
|---|---|---Extract messages (): 6.465446s [77.464073%]
|---|---|---|---graph_message::update_message init (): 0.001417s [0.021923%]
|---|---|---|---resizing messages (): 0.199745s [3.089419%]
|---|---|---|---initializing messages (): 0.187164s [2.894831%]
|---|---|---|---updating messages (): 6.012073s [92.987755%]
|---|---|---|---* symmetrizing (): 0.030768s [0.475881%]
|---|---|---|---setting message_mark and is_star_message (): 0.034264s [0.529948%]
|---|---|---colored_graph::init init (): 0.225783s [2.705163%]
|---|---|---updating adj_list (): 0.389235s [4.663514%]
|---|---|---Find deg and ver_types (): 1.265874s [15.166743%]
|---|---Encode * vertices (): 2.294833s [12.773107%]
|---|---Encode * edges (): 0.000052s [0.000290%]
|---|---Encode vertex types (): 2.315811s [12.889873%]
|---|---Extract partition graphs (): 2.749254s [15.302428%]
|---|---Encode partition b graphs (): 2.082472s [11.591098%]
|---|---Encode partition graphs (): 0.174702s [0.972397%]
|---Decode (): 12.768882s [36.864243%]
|---|---Init (): 0.000014s [0.000111%]
|---|---Decode * vertices (): 2.372627s [18.581320%]
|---|---Decode * edges (): 0.000034s [0.000268%]
|---|---Decode vertex types (): 6.345600s [49.695812%]
|---|---Decode partition graphs (): 0.000051s [0.000399%]
|---|---Decode partition b graphs (): 3.525432s [27.609560%]
|---|---Construct decoded graph (): 0.525092s [4.112280%]
|---compare (): 2.957078s [8.537197%]
       34.64 real        33.47 user         0.72 sys
```

</details>


<details>
<summary> after </summary>

```
|---Construct G () 2019-04-09 05:05:10 PM
 edges size 300470
 graph constructed
|---Encode () 2019-04-09 05:05:11 PM
|---|---Init compressed () 2019-04-09 05:05:11 PM
|---|---Extact edge types () 2019-04-09 05:05:11 PM
|---|---|---Extract messages () 2019-04-09 05:05:11 PM
|---|---|---|---graph_message::update_message init () 2019-04-09 05:05:11 PM
|---|---|---|---resizing messages () 2019-04-09 05:05:11 PM
|---|---|---|---initializing messages () 2019-04-09 05:05:11 PM
|---|---|---|---updating messages () 2019-04-09 05:05:11 PM
 total time to search in hash table: 0.755252
 total time to insert in hash table: 1.52036
 total time to modify vector m  0.79718
 total time to sort  0.07528
 total time to collect neighbor messages 0.752213
|---|---|---|---* symmetrizing () 2019-04-09 05:05:17 PM
|---|---|---|---setting message_mark and is_star_message () 2019-04-09 05:05:17 PM
|---|---|---colored_graph::init init () 2019-04-09 05:05:17 PM
|---|---|---updating adj_list () 2019-04-09 05:05:17 PM
|---|---|---Find deg and ver_types () 2019-04-09 05:05:18 PM
|---|---Encode * vertices () 2019-04-09 05:05:19 PM
|---|---Encode * edges () 2019-04-09 05:05:21 PM
|---|---Encode vertex types () 2019-04-09 05:05:21 PM
|---|---Extract partition graphs () 2019-04-09 05:05:24 PM
|---|---Encode partition b graphs () 2019-04-09 05:05:26 PM
|---|---Encode partition graphs () 2019-04-09 05:05:28 PM
|---Decode () 2019-04-09 05:05:29 PM
|---|---Init () 2019-04-09 05:05:29 PM
|---|---Decode * vertices () 2019-04-09 05:05:29 PM
|---|---Decode * edges () 2019-04-09 05:05:31 PM
|---|---Decode vertex types () 2019-04-09 05:05:31 PM
|---|---Decode partition graphs () 2019-04-09 05:05:38 PM
|---|---Decode partition b graphs () 2019-04-09 05:05:38 PM
|---|---Construct decoded graph () 2019-04-09 05:05:41 PM
|---compare () 2019-04-09 05:05:42 PM
 successfully decoded the marked graph :D

|---Construct G (): 0.956205s [2.775045%]
|---Encode (): 17.612391s [51.113735%]
|---|---Init compressed (): 0.000653s [0.003709%]
|---|---Extact edge types (): 7.961563s [45.204330%]
|---|---|---Extract messages (): 6.084527s [76.423775%]
|---|---|---|---graph_message::update_message init (): 0.001444s [0.023740%]
|---|---|---|---resizing messages (): 0.192311s [3.160649%]
|---|---|---|---initializing messages (): 0.187849s [3.087325%]
|---|---|---|---updating messages (): 5.632613s [92.572739%]
|---|---|---|---* symmetrizing (): 0.032790s [0.538907%]
|---|---|---|---setting message_mark and is_star_message (): 0.037503s [0.616363%]
|---|---|---colored_graph::init init (): 0.214541s [2.694708%]
|---|---|---updating adj_list (): 0.395865s [4.972202%]
|---|---|---Find deg and ver_types (): 1.266586s [15.908758%]
|---|---Encode * vertices (): 2.331242s [13.236377%]
|---|---Encode * edges (): 0.000055s [0.000311%]
|---|---Encode vertex types (): 2.293806s [13.023817%]
|---|---Extract partition graphs (): 2.750105s [15.614605%]
|---|---Encode partition b graphs (): 2.096790s [11.905195%]
|---|---Encode partition graphs (): 0.176885s [1.004324%]
|---Decode (): 12.922110s [37.501854%]
|---|---Init (): 0.000021s [0.000161%]
|---|---Decode * vertices (): 2.465586s [19.080364%]
|---|---Decode * edges (): 0.000037s [0.000283%]
|---|---Decode vertex types (): 6.388016s [49.434776%]
|---|---Decode partition graphs (): 0.000078s [0.000603%]
|---|---Decode partition b graphs (): 3.535769s [27.362165%]
|---|---Construct decoded graph (): 0.532570s [4.121383%]
|---compare (): 2.966536s [8.609321%]
       34.46 real        33.37 user         0.72 sys
```

</details>


## 2019-04-09 9:17 PM

Realized that it is not really necessary to work with `adj_location` which is a complex map structure in marked graphs. Instead, defined `index_in_neighbor`: for a vertex v, `index_in_neighbor[v][i]` is the index of v in the adjacency list of the ith neighbor of v. With this, only modifying update message, I had a ~0.5 s improvement: 

## 2019-04-10 11:50 AM

Removed the attribute `adj_location` from both marked graph and colored graph, and used `index_in_neighbor` in all places. This results in ~2 s improvement: 

<details>

<summary> details </summary>

```
|---Construct G () 2019-04-10 11:43:03 AM
 edges size 300440
 graph constructed
|---Encode () 2019-04-10 11:43:04 AM
|---|---Init compressed () 2019-04-10 11:43:04 AM
|---|---Extact edge types () 2019-04-10 11:43:04 AM
|---|---|---Extract messages () 2019-04-10 11:43:04 AM
|---|---|---|---graph_message::update_message init () 2019-04-10 11:43:04 AM
|---|---|---|---resizing messages () 2019-04-10 11:43:04 AM
|---|---|---|---initializing messages () 2019-04-10 11:43:04 AM
|---|---|---|---updating messages () 2019-04-10 11:43:04 AM
 total time to search in hash table: 0.760559
 total time to insert in hash table: 1.55026
 total time to modify vector m  0.784155
 total time to sort  0.0688325
 total time to collect neighbor messages 0.41188
|---|---|---|---* symmetrizing () 2019-04-10 11:43:10 AM
|---|---|---|---setting message_mark and is_star_message () 2019-04-10 11:43:10 AM
|---|---|---colored_graph::init init () 2019-04-10 11:43:10 AM
|---|---|---updating adj_list () 2019-04-10 11:43:10 AM
|---|---|---Find deg and ver_types () 2019-04-10 11:43:10 AM
|---|---Encode * vertices () 2019-04-10 11:43:11 AM
|---|---Encode * edges () 2019-04-10 11:43:13 AM
|---|---Encode vertex types () 2019-04-10 11:43:13 AM
|---|---Extract partition graphs () 2019-04-10 11:43:16 AM
|---|---Encode partition b graphs () 2019-04-10 11:43:18 AM
|---|---Encode partition graphs () 2019-04-10 11:43:20 AM
|---Decode () 2019-04-10 11:43:21 AM
|---|---Init () 2019-04-10 11:43:21 AM
|---|---Decode * vertices () 2019-04-10 11:43:21 AM
|---|---Decode * edges () 2019-04-10 11:43:23 AM
|---|---Decode vertex types () 2019-04-10 11:43:23 AM
|---|---Decode partition graphs () 2019-04-10 11:43:29 AM
|---|---Decode partition b graphs () 2019-04-10 11:43:29 AM
|---|---Construct decoded graph () 2019-04-10 11:43:32 AM
|---compare () 2019-04-10 11:43:33 AM
 successfully decoded the marked graph :D

|---Construct G (): 0.829669s [2.594985%]
|---Encode (): 16.433228s [51.398804%]
|---|---Init compressed (): 0.000718s [0.004366%]
|---|---Extact edge types (): 7.172252s [43.644817%]
|---|---|---Extract messages (): 5.716563s [79.703888%]
|---|---|---|---graph_message::update_message init (): 0.001409s [0.024649%]
|---|---|---|---resizing messages (): 0.191876s [3.356501%]
|---|---|---|---initializing messages (): 0.179843s [3.145999%]
|---|---|---|---updating messages (): 5.273019s [92.241066%]
|---|---|---|---* symmetrizing (): 0.032299s [0.565013%]
|---|---|---|---setting message_mark and is_star_message (): 0.037780s [0.660887%]
|---|---|---colored_graph::init init (): 0.024582s [0.342731%]
|---|---|---updating adj_list (): 0.220640s [3.076303%]
|---|---|---Find deg and ver_types (): 1.210425s [16.876507%]
|---|---Encode * vertices (): 2.164104s [13.169074%]
|---|---Encode * edges (): 0.000043s [0.000263%]
|---|---Encode vertex types (): 2.158486s [13.134887%]
|---|---Extract partition graphs (): 2.701006s [16.436249%]
|---|---Encode partition b graphs (): 2.064137s [12.560755%]
|---|---Encode partition graphs (): 0.171202s [1.041805%]
|---Decode (): 12.170957s [38.067539%]
|---|---Init (): 0.000023s [0.000190%]
|---|---Decode * vertices (): 2.286243s [18.784418%]
|---|---Decode * edges (): 0.000034s [0.000280%]
|---|---Decode vertex types (): 6.092728s [50.059563%]
|---|---Decode partition graphs (): 0.000061s [0.000503%]
|---|---Decode partition b graphs (): 3.370534s [27.693254%]
|---|---Construct decoded graph (): 0.421302s [3.461539%]
|---compare (): 2.538139s [7.938629%]
       31.98 real        31.27 user         0.58 sys
```


</details>

## 2019-04-12 7:04 PM

I realized that the inputs to the `compute_product` function do not need to be of type `mpz_class`, only `int` is enough. This surprisingly results in about 6s improvement!

<details>

<summary> details </summary>

Note that I commented the parts to compute total time to search in hash table etc. 

```
|---Construct G () 2019-04-12 07:00:18 PM
 edges size 300457
 graph constructed
|---Encode () 2019-04-12 07:00:19 PM
|---|---Init compressed () 2019-04-12 07:00:19 PM
|---|---Extact edge types () 2019-04-12 07:00:19 PM
|---|---|---Extract messages () 2019-04-12 07:00:19 PM
|---|---|---|---graph_message::update_message init () 2019-04-12 07:00:19 PM
|---|---|---|---resizing messages () 2019-04-12 07:00:19 PM
|---|---|---|---initializing messages () 2019-04-12 07:00:19 PM
|---|---|---|---updating messages () 2019-04-12 07:00:19 PM
 total time to search in hash table: 0
 total time to insert in hash table: 0
 total time to modify vector m  0
 total time to sort  0
 total time to collect neighbor messages 0
|---|---|---|---* symmetrizing () 2019-04-12 07:00:23 PM
|---|---|---|---setting message_mark and is_star_message () 2019-04-12 07:00:23 PM
|---|---|---colored_graph::init init () 2019-04-12 07:00:23 PM
|---|---|---updating adj_list () 2019-04-12 07:00:23 PM
|---|---|---Find deg and ver_types () 2019-04-12 07:00:23 PM
|---|---Encode * vertices () 2019-04-12 07:00:25 PM
|---|---Encode * edges () 2019-04-12 07:00:26 PM
|---|---Encode vertex types () 2019-04-12 07:00:26 PM
|---|---Extract partition graphs () 2019-04-12 07:00:27 PM
|---|---Encode partition b graphs () 2019-04-12 07:00:30 PM
|---|---Encode partition graphs () 2019-04-12 07:00:32 PM
|---Decode () 2019-04-12 07:00:32 PM
|---|---Init () 2019-04-12 07:00:32 PM
|---|---Decode * vertices () 2019-04-12 07:00:32 PM
|---|---Decode * edges () 2019-04-12 07:00:33 PM
|---|---Decode vertex types () 2019-04-12 07:00:33 PM
|---|---Decode partition graphs () 2019-04-12 07:00:37 PM
|---|---Decode partition b graphs () 2019-04-12 07:00:37 PM
|---|---Construct decoded graph () 2019-04-12 07:00:40 PM
|---compare () 2019-04-12 07:00:41 PM
 successfully decoded the marked graph :D

|---Construct G (): 0.838441s [3.293217%]
|---Encode (): 13.342818s [52.407761%]
|---|---Init compressed (): 0.000688s [0.005153%]
|---|---Extact edge types (): 6.134593s [45.976738%]
|---|---|---Extract messages (): 4.592058s [74.855133%]
|---|---|---|---graph_message::update_message init (): 0.001506s [0.032806%]
|---|---|---|---resizing messages (): 0.194764s [4.241329%]
|---|---|---|---initializing messages (): 0.193184s [4.206914%]
|---|---|---|---updating messages (): 4.128408s [89.903221%]
|---|---|---|---* symmetrizing (): 0.034695s [0.755551%]
|---|---|---|---setting message_mark and is_star_message (): 0.039188s [0.853394%]
|---|---|---colored_graph::init init (): 0.027730s [0.452019%]
|---|---|---updating adj_list (): 0.236458s [3.854509%]
|---|---|---Find deg and ver_types (): 1.278302s [20.837597%]
|---|---Encode * vertices (): 1.130349s [8.471589%]
|---|---Encode * edges (): 0.000045s [0.000334%]
|---|---Encode vertex types (): 1.154480s [8.652441%]
|---|---Extract partition graphs (): 2.733744s [20.488504%]
|---|---Encode partition b graphs (): 2.016023s [15.109422%]
|---|---Encode partition graphs (): 0.171038s [1.281871%]
|---Decode (): 8.703982s [34.187397%]
|---|---Init (): 0.000019s [0.000223%]
|---|---Decode * vertices (): 1.197001s [13.752331%]
|---|---Decode * edges (): 0.000042s [0.000481%]
|---|---Decode vertex types (): 3.619471s [41.584076%]
|---|---Decode partition graphs (): 0.000045s [0.000514%]
|---|---Decode partition b graphs (): 3.433578s [39.448360%]
|---|---Construct decoded graph (): 0.453796s [5.213660%]
|---compare (): 2.574372s [10.111587%]
       25.47 real        24.60 user         0.60 sys
```

</details>



## 2019-04-12 7:12 PM

I also tried to see if another way of splitting the terms for `compute_product` can result in an improvement. The other form (see handwritten note for details) is as follows:

$$ \prod_{i=0}^{k-1} (r - is) = \prod_{i=0}^{\lceil k / 2 \rceil - 1} (r - i(2s)) \times \prod_{i=0}^{\lfloor k / 2 \rfloor - 1} ((r-s) - i(2s))$$

Which means that $\text{compute_product}(r, k, s) = \text{compute_product}(r, m, 2s) \times \text{compute_product}(r-s, k-m, 2s)$ where $m = (k+1) \div 2$. I tested this, but did not see an improvement. I expected one, because this is a more balanced split. 

<details>

<summary> details </summary>

```
|---Construct G () 2019-04-12 07:11:21 PM
 edges size 299838
 graph constructed
|---Encode () 2019-04-12 07:11:21 PM
|---|---Init compressed () 2019-04-12 07:11:21 PM
|---|---Extact edge types () 2019-04-12 07:11:21 PM
|---|---|---Extract messages () 2019-04-12 07:11:21 PM
|---|---|---|---graph_message::update_message init () 2019-04-12 07:11:21 PM
|---|---|---|---resizing messages () 2019-04-12 07:11:21 PM
|---|---|---|---initializing messages () 2019-04-12 07:11:22 PM
|---|---|---|---updating messages () 2019-04-12 07:11:22 PM
 total time to search in hash table: 0
 total time to insert in hash table: 0
 total time to modify vector m  0
 total time to sort  0
 total time to collect neighbor messages 0
|---|---|---|---* symmetrizing () 2019-04-12 07:11:26 PM
|---|---|---|---setting message_mark and is_star_message () 2019-04-12 07:11:26 PM
|---|---|---colored_graph::init init () 2019-04-12 07:11:26 PM
|---|---|---updating adj_list () 2019-04-12 07:11:26 PM
|---|---|---Find deg and ver_types () 2019-04-12 07:11:26 PM
|---|---Encode * vertices () 2019-04-12 07:11:28 PM
|---|---Encode * edges () 2019-04-12 07:11:29 PM
|---|---Encode vertex types () 2019-04-12 07:11:29 PM
|---|---Extract partition graphs () 2019-04-12 07:11:30 PM
|---|---Encode partition b graphs () 2019-04-12 07:11:33 PM
|---|---Encode partition graphs () 2019-04-12 07:11:35 PM
|---Decode () 2019-04-12 07:11:35 PM
|---|---Init () 2019-04-12 07:11:35 PM
|---|---Decode * vertices () 2019-04-12 07:11:35 PM
|---|---Decode * edges () 2019-04-12 07:11:36 PM
|---|---Decode vertex types () 2019-04-12 07:11:36 PM
|---|---Decode partition graphs () 2019-04-12 07:11:40 PM
|---|---Decode partition b graphs () 2019-04-12 07:11:40 PM
|---|---Construct decoded graph () 2019-04-12 07:11:43 PM
|---compare () 2019-04-12 07:11:43 PM
 successfully decoded the marked graph :D

|---Construct G (): 0.846560s [3.329669%]
|---Encode (): 13.309370s [52.348114%]
|---|---Init compressed (): 0.000787s [0.005912%]
|---|---Extact edge types (): 6.099671s [45.829899%]
|---|---|---Extract messages (): 4.611472s [75.601974%]
|---|---|---|---graph_message::update_message init (): 0.001362s [0.029537%]
|---|---|---|---resizing messages (): 0.192514s [4.174665%]
|---|---|---|---initializing messages (): 0.190769s [4.136829%]
|---|---|---|---updating messages (): 4.160253s [90.215286%]
|---|---|---|---* symmetrizing (): 0.029564s [0.641104%]
|---|---|---|---setting message_mark and is_star_message (): 0.036786s [0.797699%]
|---|---|---colored_graph::init init (): 0.025158s [0.412448%]
|---|---|---updating adj_list (): 0.228830s [3.751512%]
|---|---|---Find deg and ver_types (): 1.234159s [20.233212%]
|---|---Encode * vertices (): 1.139122s [8.558794%]
|---|---Encode * edges (): 0.000050s [0.000373%]
|---|---Encode vertex types (): 1.177471s [8.846930%]
|---|---Extract partition graphs (): 2.741612s [20.599110%]
|---|---Encode partition b graphs (): 1.975690s [14.844354%]
|---|---Encode partition graphs (): 0.173620s [1.304497%]
|---Decode (): 8.671961s [34.108360%]
|---|---Init (): 0.000020s [0.000227%]
|---|---Decode * vertices (): 1.194511s [13.774403%]
|---|---Decode * edges (): 0.000037s [0.000429%]
|---|---Decode vertex types (): 3.631156s [41.872372%]
|---|---Decode partition graphs (): 0.000049s [0.000569%]
|---|---Decode partition b graphs (): 3.408437s [39.304111%]
|---|---Construct decoded graph (): 0.437722s [5.047554%]
|---compare (): 2.596835s [10.213815%]
       25.43 real        24.67 user         0.57 sys
```
</details>


## 2019-04-16 5:40 PM


Realized that there is a more efficient way of updating messages: when we gather previous messages and sort them, we do not need to construct a vector for each neighbor separately, we can swap the last element one by one to find the message. This is done by working with capacity and size, so that we keep the element of the message that we do not need at the end of the vector outside its size so that we can use it later by swapping.

Also, I realized that we do not need to keep a list of all messages, the only information we need is whether a message is star type and its mark component.

Also, we do not need to keep a copy of message_dict and messages for all depths, as at depth s we only need depth s-1, so at each depth,  we can get a backup of current variables and clear them and update them using this backup (which refers to the previous depth). 

With all these, I seem to have an improvement of around 1s, but I expected at least 3s! Now, for 100,000 nodes, 300,000 edges, h = 3, delta = 20. Here is details:


<details>

<summary> details </summary>

```
|---Construct G () 2019-04-16 05:28:48 PM
 edges size 299124
 graph constructed
|---Encode () 2019-04-16 05:28:49 PM
|---|---Init compressed () 2019-04-16 05:28:49 PM
|---|---Extact edge types () 2019-04-16 05:28:49 PM
|---|---|---Extract messages () 2019-04-16 05:28:49 PM
|---|---|---|---graph_message::update_message init () 2019-04-16 05:28:49 PM
|---|---|---|---resizing messages () 2019-04-16 05:28:49 PM
|---|---|---|---initializing messages () 2019-04-16 05:28:49 PM
|---|---|---|---updating messages () 2019-04-16 05:28:49 PM
|---|---|---|---* symmetrizing () 2019-04-16 05:28:53 PM
|---|---|---colored_graph::init init () 2019-04-16 05:28:53 PM
|---|---|---updating adj_list () 2019-04-16 05:28:53 PM
|---|---|---Find deg and ver_types () 2019-04-16 05:28:53 PM
|---|---Encode * vertices () 2019-04-16 05:28:54 PM
|---|---Encode * edges () 2019-04-16 05:28:56 PM
|---|---Encode vertex types () 2019-04-16 05:28:56 PM
|---|---Extract partition graphs () 2019-04-16 05:28:57 PM
|---|---Encode partition b graphs () 2019-04-16 05:29:00 PM
|---|---Encode partition graphs () 2019-04-16 05:29:01 PM
|---Decode () 2019-04-16 05:29:02 PM
|---|---Init () 2019-04-16 05:29:02 PM
|---|---Decode * vertices () 2019-04-16 05:29:02 PM
|---|---Decode * edges () 2019-04-16 05:29:03 PM
|---|---Decode vertex types () 2019-04-16 05:29:03 PM
|---|---Decode partition graphs () 2019-04-16 05:29:06 PM
|---|---Decode partition b graphs () 2019-04-16 05:29:06 PM
|---|---Construct decoded graph () 2019-04-16 05:29:10 PM
|---compare () 2019-04-16 05:29:10 PM
 successfully decoded the marked graph :D

|---Construct G (): 0.853117s [3.544942%]
|---Encode (): 12.856951s [53.424271%]
|---|---Init compressed (): 0.000504s [0.003918%]
|---|---Extact edge types (): 5.701544s [44.346008%]
|---|---|---Extract messages (): 4.292262s [75.282448%]
|---|---|---|---graph_message::update_message init (): 0.001364s [0.031769%]
|---|---|---|---resizing messages (): 0.025791s [0.600883%]
|---|---|---|---initializing messages (): 0.179556s [4.183252%]
|---|---|---|---updating messages (): 3.769229s [87.814514%]
|---|---|---|---* symmetrizing (): 0.316306s [7.369211%]
|---|---|---colored_graph::init init (): 0.022788s [0.399686%]
|---|---|---updating adj_list (): 0.163689s [2.870965%]
|---|---|---Find deg and ver_types (): 1.222781s [21.446480%]
|---|---Encode * vertices (): 1.128720s [8.779068%]
|---|---Encode * edges (): 0.000044s [0.000342%]
|---|---Encode vertex types (): 1.150674s [8.949819%]
|---|---Extract partition graphs (): 2.728767s [21.224064%]
|---|---Encode partition b graphs (): 1.966031s [15.291584%]
|---|---Encode partition graphs (): 0.179268s [1.394325%]
|---Decode (): 8.595399s [35.716316%]
|---|---Init (): 0.000018s [0.000204%]
|---|---Decode * vertices (): 1.209178s [14.067740%]
|---|---Decode * edges (): 0.000051s [0.000593%]
|---|---Decode vertex types (): 3.569625s [41.529484%]
|---|---Decode partition graphs (): 0.000054s [0.000633%]
|---|---Decode partition b graphs (): 3.373827s [39.251541%]
|---|---Construct decoded graph (): 0.442614s [5.149432%]
|---compare (): 1.760271s [7.314426%]
       24.07 real        23.38 user         0.50 sys
```

</details>


I am starting to think that perhaps I am expecting too much. This is somehow the maximum entropy ensemble and perhaps that's why it takes this much time. For other types of data, e.g. more symmetric ones, perhaps we can be faster. The reason for this: among the ~300,000 edges and 600,000 half edges, almost all of them have a unique type, more specifically, in an example, the graph has 299,768 edges and 598,135 type, so nearly the maximum, and this is because there is no symmetry in the graph. Maybe I should run it and test it in different examples. Yet the idea of multithreading exists, but that might be a little difficult. 

Commit 811aa34.

## 2019-04-18 11:40 AM

I want to test the code on a non ER example, e.g. a regular graph, to see how long it takes to encode. 

I also looked into WebGraph to see if I can find any implementation, and see if I con compare with it. Here are some perhaps useful links:

- [WebGraph homepage](http://webgraph.di.unimi.it/)
- [A C++ implementation](http://cnets.indiana.edu/groups/nan/webgraph/)
- [The GitHub page for the C++ implementation](https://github.com/jacobratkiewicz/webgraph)


## 2019-04-18 12:29 PM

I implemented a version of regular graph which generates a graph which is nearly regular (`random_graph/near_regular_graph(...)`). I tested compression with this, and it is still ~20s. In this regime, I do not have marks, so that message passing is fast, but compressing partition graphs is more difficult, as there are ~25,000 types and more partition graphs. The graph has 100,000 nodes, half degree is 3, so the average degree is ~ 6, and the number of edges is ~300,000, h = 3, delta = 20, overall time ~22 s.

<details>

<summary> details </summary>

```
|---Construct G () 2019-04-18 12:31:48 PM
 graph constructed
|---Encode () 2019-04-18 12:31:49 PM
|---|---Init compressed () 2019-04-18 12:31:49 PM
|---|---Extact edge types () 2019-04-18 12:31:49 PM
|---|---|---Extract messages () 2019-04-18 12:31:49 PM
|---|---|---|---graph_message::update_message init () 2019-04-18 12:31:49 PM
|---|---|---|---resizing messages () 2019-04-18 12:31:49 PM
|---|---|---|---initializing messages () 2019-04-18 12:31:49 PM
|---|---|---|---updating messages () 2019-04-18 12:31:49 PM
|---|---|---|---* symmetrizing () 2019-04-18 12:31:51 PM
|---|---|---colored_graph::init init () 2019-04-18 12:31:51 PM
|---|---|---updating adj_list () 2019-04-18 12:31:51 PM
|---|---|---Find deg and ver_types () 2019-04-18 12:31:51 PM
 number of types 24971
|---|---Encode * vertices () 2019-04-18 12:31:52 PM
|---|---Encode * edges () 2019-04-18 12:31:53 PM
|---|---Encode vertex types () 2019-04-18 12:31:53 PM
|---|---Extract partition graphs () 2019-04-18 12:31:54 PM
|---|---Encode partition b graphs () 2019-04-18 12:31:58 PM
|---|---Encode partition graphs () 2019-04-18 12:32:00 PM
|---Decode () 2019-04-18 12:32:00 PM
|---|---Init () 2019-04-18 12:32:00 PM
|---|---Decode * vertices () 2019-04-18 12:32:00 PM
|---|---Decode * edges () 2019-04-18 12:32:01 PM
|---|---Decode vertex types () 2019-04-18 12:32:01 PM
|---|---Decode partition graphs () 2019-04-18 12:32:06 PM
|---|---Decode partition b graphs () 2019-04-18 12:32:06 PM
|---|---Construct decoded graph () 2019-04-18 12:32:09 PM
|---compare () 2019-04-18 12:32:10 PM
 successfully decoded the marked graph :D

|---Construct G (): 0.778341s [3.410910%]
|---Encode (): 10.788221s [47.277035%]
|---|---Init compressed (): 0.000678s [0.006282%]
|---|---Extact edge types (): 2.875751s [26.656393%]
|---|---|---Extract messages (): 1.559093s [54.215176%]
|---|---|---|---graph_message::update_message init (): 0.002387s [0.153073%]
|---|---|---|---resizing messages (): 0.026026s [1.669308%]
|---|---|---|---initializing messages (): 0.152151s [9.758941%]
|---|---|---|---updating messages (): 1.284092s [82.361481%]
|---|---|---|---* symmetrizing (): 0.094420s [6.056058%]
|---|---|---colored_graph::init init (): 0.025348s [0.881431%]
|---|---|---updating adj_list (): 0.161420s [5.613137%]
|---|---|---Find deg and ver_types (): 1.129845s [39.288700%]
|---|---Encode * vertices (): 1.139338s [10.560946%]
|---|---Encode * edges (): 0.000044s [0.000405%]
|---|---Encode vertex types (): 1.179079s [10.929317%]
|---|---Extract partition graphs (): 3.359696s [31.142258%]
|---|---Encode partition b graphs (): 2.062739s [19.120291%]
|---|---Encode partition graphs (): 0.169248s [1.568820%]
|---Decode (): 9.480145s [41.544674%]
|---|---Init (): 0.000008s [0.000087%]
|---|---Decode * vertices (): 1.263069s [13.323313%]
|---|---Decode * edges (): 0.000074s [0.000778%]
|---|---Decode vertex types (): 4.252406s [44.855915%]
|---|---Decode partition graphs (): 0.003155s [0.033284%]
|---|---Decode partition b graphs (): 3.423949s [36.117058%]
|---|---Construct decoded graph (): 0.537452s [5.669237%]
|---compare (): 1.772439s [7.767329%]
       22.82 real        21.98 user         0.43 sys
```

</details>


If I want to improve this even more, I think I should go for parallel programming: there are two places that we can do parallel computing: extract messages (as different nodes act independently) and compression of partition graphs (each partition graph has independent calculations). I think this can be pretty useful. But I want to first implement the binarization and count the actual number of bits and draw a plot that compares it with the actual entropy. 

Also, [this](https://arxiv.org/pdf/1006.0809.pdf) paper might be useful, as it has some plots that compares time and compression rate for different methods. I can see that the order of time is microseconds per edge! With this, I have ~20 seconds for ~ 300,000 edges, which is ~73 microseconds per edge, but they have ~ 1 microseconds / edge. But we are compressing different graphs. Perhaps I can somehow understand what graphs they use and just do a comparison. This will take some time.

## 2019-04-19 

I converted messages to strings to see if I get an improvement, but it actually made it worse (~ 24s for ER 100,000 example). So I think I will revert. In this new messaging protocol, I also include the mark from v to w in the message v sends to w, so that when aggregating messages, I do not have to add this as well, which makes the string even longer. 

This is commit fc3792f, branch `mp_improve_string_message`

<details>

<summary> details </summary>

```
|---Construct G () 2019-04-24 06:42:19 PM
 edges size 299643
 graph constructed
|---Encode () 2019-04-24 06:42:20 PM
|---|---Init compressed () 2019-04-24 06:42:20 PM
|---|---Extact edge types () 2019-04-24 06:42:20 PM
|---|---|---Extract messages () 2019-04-24 06:42:20 PM
|---|---|---|---graph_message::update_message init () 2019-04-24 06:42:20 PM
|---|---|---|---resizing messages () 2019-04-24 06:42:20 PM
|---|---|---|---initializing messages () 2019-04-24 06:42:20 PM
|---|---|---|---updating messages () 2019-04-24 06:42:20 PM
|---|---|---|---* symmetrizing () 2019-04-24 06:42:25 PM
|---|---|---colored_graph::init init () 2019-04-24 06:42:25 PM
|---|---|---updating adj_list () 2019-04-24 06:42:25 PM
|---|---|---Find deg and ver_types () 2019-04-24 06:42:25 PM
 number of types 597453
|---|---Encode * vertices () 2019-04-24 06:42:27 PM
|---|---Encode * edges () 2019-04-24 06:42:28 PM
|---|---Encode vertex types () 2019-04-24 06:42:28 PM
|---|---Extract partition graphs () 2019-04-24 06:42:29 PM
|---|---Encode partition b graphs () 2019-04-24 06:42:32 PM
|---|---Encode partition graphs () 2019-04-24 06:42:34 PM
|---Decode () 2019-04-24 06:42:34 PM
|---|---Init () 2019-04-24 06:42:34 PM
|---|---Decode * vertices () 2019-04-24 06:42:34 PM
|---|---Decode * edges () 2019-04-24 06:42:35 PM
|---|---Decode vertex types () 2019-04-24 06:42:35 PM
|---|---Decode partition graphs () 2019-04-24 06:42:39 PM
|---|---Decode partition b graphs () 2019-04-24 06:42:39 PM
|---|---Construct decoded graph () 2019-04-24 06:42:42 PM
|---compare () 2019-04-24 06:42:42 PM
 successfully decoded the marked graph :D

|---Construct G (): 0.883083s [3.524292%]
|---Encode (): 13.988238s [55.825581%]
|---|---Init compressed (): 0.000028s [0.000201%]
|---|---Extact edge types (): 6.800817s [48.618107%]
|---|---|---Extract messages (): 5.342340s [78.554382%]
|---|---|---|---graph_message::update_message init (): 0.001285s [0.024046%]
|---|---|---|---resizing messages (): 0.025195s [0.471612%]
|---|---|---|---initializing messages (): 0.540789s [10.122693%]
|---|---|---|---updating messages (): 4.687753s [87.747177%]
|---|---|---|---* symmetrizing (): 0.087298s [1.634081%]
|---|---|---colored_graph::init init (): 0.022202s [0.326454%]
|---|---|---updating adj_list (): 0.160734s [2.363451%]
|---|---|---Find deg and ver_types (): 1.275526s [18.755480%]
|---|---Encode * vertices (): 1.101893s [7.877285%]
|---|---Encode * edges (): 0.000052s [0.000372%]
|---|---Encode vertex types (): 1.151198s [8.229755%]
|---|---Extract partition graphs (): 2.824238s [20.190092%]
|---|---Encode partition b graphs (): 1.932755s [13.817003%]
|---|---Encode partition graphs (): 0.177187s [1.266684%]
|---Decode (): 8.496358s [33.908066%]
|---|---Init (): 0.000008s [0.000093%]
|---|---Decode * vertices (): 1.216012s [14.312156%]
|---|---Decode * edges (): 0.000051s [0.000603%]
|---|---Decode vertex types (): 3.612415s [42.517220%]
|---|---Decode partition graphs (): 0.000160s [0.001878%]
|---|---Decode partition b graphs (): 3.224521s [37.951805%]
|---|---Construct decoded graph (): 0.443159s [5.215867%]
|---compare (): 1.689352s [6.742027%]

real	0m25.070s
user	0m24.252s
sys	0m0.486s
```

</details>


## 2019-04-29

On binarize branch. Realized last week that in order to binarize star edges, I need some work, as there is no simple bit stream data structure in C as bit is not a unit of data, byte is a unit of data. For this purpose, I did some search today to see the options: 

- There is [bitset](http://www.cplusplus.com/reference/bitset/bitset/) data structure in C. The difficulty is that it requires a fixed size at construction. 
- Apparently boost has a [dynamic bitset](https://www.boost.org/doc/libs/1_36_0/libs/dynamic_bitset/dynamic_bitset.html), but did not understand how it works. 
- There is [this](http://stanford.edu/~stepp/cppdoc/) Stanford C++ libraries, which can be useful perhaps). 
- I can also work with strings of zero ones and stringstreams. 

I think what I am going to do is the following (see test code 13): 

- Use bitset with size $\lceil \log_2 n\rceil$ to convert integer to specific number of bits, and append it to a string using `bitset::to_string`.
- The resulting string of zero and ones can be converted back to bitstream of size 8 (one byte) using stringstream and could be written to file. 

I need to test this and add it to the project, but perhaps later. 

## 2019-05-17 

Trying to clean up the code to make it easy for experiments. Now, I am learning about option parsers. One option is Boost and the other is an old one called getopt. [This](https://coderwall.com/p/y3xnxg/using-getopt-vs-boost-in-c-to-handle-arguments) is a nice short comparison. I tried Boost but it gave me linker errors, maybe my boost is not installed correctly? I think I will try to use getopt. 

## 2019-05-29

I am trying to download webgraph examples, and see how my algorithm performs in those examples. But I am facing difficulties. First of all, the webgraph java file downloaded from [maven](https://search.maven.org/search?q=g:it.unimi.dsi) site (that was mentioned in the [webgraph](http://webgraph.di.unimi.it/) site) does not work. More specifically, when I try `java -jar webgraph-3.6.1.jar`, I get the error `no main manifest attribute, in webgraph-3.6.1.jar`. I also downloaded the c++ version ([webgraph++](https://cnets.indiana.edu/groups/nan/webgraph/)) from [here](https://github.com/jacobratkiewicz/webgraph), but when I try to make it, I receive Boost errors (`fatal error: 'boost/iterator/iterator_facade.hpp' file not found`). I tried to dig through a sample graph database, for instance the eu-2005 graph which can be found in [this](http://law.di.unimi.it/webdata/eu-2005/) page. This graph has ~800k nodes and 19M edges. But the .graph file seems to be binary, and I could not parse it. 

## 2019-05-30 

I experimented a little bit regarding information theoretic optimality, i.e. the length of the compressed form. In order to do this, I did the following:

- generated a Poisson random graph with $n = 10^6$ nodes, $d = 2$, $2001001$ edges and no marks (vertex and edge mark set size 1).
- compressed using $h = 1$ and $d = 10$. 

<details> 

<summary> compression timing </summary>

```
|---Init compressed () 2019-05-30 04:21:24 PM
|---Extact edge types () 2019-05-30 04:21:24 PM
|---|---Extract messages () 2019-05-30 04:21:24 PM
|---|---|---graph_message::update_message init () 2019-05-30 04:21:24 PM
|---|---|---resizing messages () 2019-05-30 04:21:24 PM
|---|---|---initializing messages () 2019-05-30 04:21:24 PM
|---|---|---updating messages () 2019-05-30 04:21:25 PM
|---|---|---* symmetrizing () 2019-05-30 04:21:25 PM
|---|---colored_graph::init init () 2019-05-30 04:21:25 PM
|---|---updating adj_list () 2019-05-30 04:21:25 PM
|---|---Find deg and ver_types () 2019-05-30 04:21:25 PM
|---Encode * vertices () 2019-05-30 04:21:26 PM
|---Encode * edges () 2019-05-30 04:21:38 PM
|---Encode vertex types () 2019-05-30 04:21:38 PM
|---Extract partition graphs () 2019-05-30 04:21:51 PM
|---Encode partition b graphs () 2019-05-30 04:21:52 PM
|---Encode partition graphs () 2019-05-30 04:21:52 PM
|---n () 2019-05-30 04:22:13 PM
|---h () 2019-05-30 04:22:13 PM
|---delta () 2019-05-30 04:22:13 PM
|---type_mark () 2019-05-30 04:22:13 PM
|---star_vertices () 2019-05-30 04:22:13 PM
|---star_edges () 2019-05-30 04:22:13 PM
|---vertex types () 2019-05-30 04:22:13 PM
|---partition bipartite graphs () 2019-05-30 04:22:13 PM
|---partition graphs () 2019-05-30 04:22:13 PM

|---Init compressed (): 0.000640s [0.001200%]
|---Extact edge types (): 1.418929s [2.661711%]
|---|---Extract messages (): 0.432765s [30.499405%]
|---|---|---graph_message::update_message init (): 0.006561s [1.516094%]
|---|---|---resizing messages (): 0.092744s [21.430672%]
|---|---|---initializing messages (): 0.164339s [37.974270%]
|---|---|---updating messages (): 0.000057s [0.013142%]
|---|---|---* symmetrizing (): 0.169049s [39.062645%]
|---|---colored_graph::init init (): 0.171702s [12.100852%]
|---|---updating adj_list (): 0.445012s [31.362545%]
|---|---Find deg and ver_types (): 0.369436s [26.036285%]
|---Encode * vertices (): 11.945025s [22.407188%]
|---Encode * edges (): 0.000032s [0.000060%]
|---Encode vertex types (): 13.403196s [25.142511%]
|---Extract partition graphs (): 0.913653s [1.713885%]
|---Encode partition b graphs (): 0.000038s [0.000072%]
|---Encode partition graphs (): 20.758921s [38.940819%]
|---n (): 0.000045s [0.000084%]
|---h (): 0.000009s [0.000016%]
|---delta (): 0.000007s [0.000014%]
|---type_mark (): 0.000008s [0.000015%]
|---star_vertices (): 0.000175s [0.000327%]
|---star_edges (): 0.000043s [0.000080%]
|---vertex types (): 0.000462s [0.000867%]
|---partition bipartite graphs (): 0.000014s [0.000026%]
|---partition graphs (): 1.121322s [2.103443%]

real	0m53.359s
user	0m51.498s
sys	0m0.929s
```

</details>

- compression took ~ 1 min. 
- decompression takes ~ 3 min. 

<details>

<summary> decompression timing </summary>

```
|---Init () 2019-05-30 04:23:28 PM
|---Decode * vertices () 2019-05-30 04:23:28 PM
|---Decode * edges () 2019-05-30 04:23:41 PM
|---Decode vertex types () 2019-05-30 04:23:41 PM
|---Decode partition graphs () 2019-05-30 04:23:58 PM
|---Decode partition b graphs () 2019-05-30 04:26:45 PM
|---Construct decoded graph () 2019-05-30 04:26:45 PM

|---Init (): 0.000628s [0.000306%]
|---Decode * vertices (): 12.993829s [6.324592%]
|---Decode * edges (): 0.000242s [0.000118%]
|---Decode vertex types (): 17.179178s [8.361760%]
|---Decode partition graphs (): 167.431870s [81.495468%]
|---Decode partition b graphs (): 0.000118s [0.000057%]
|---Construct decoded graph (): 7.836465s [3.814306%]

real	3m25.487s
user	3m15.511s
sys	0m4.333s
```

</details>

- the size of the compressed file is 4889783 bytes. 
- I assume that the local weak limit is Poisson Galton Watson with average degree $2 \times d = 4$. Therefore, the length of compressed form must be around $m \log_2 n + s(2d) n$. Substituting with $m = 2001001$, $n = 10^6$ and $d =2$, and then dividing by 8 (to convert bits to bytes) we get 4888812 bytes, which is pretty close to the actual file size (4889783). 

## 2019-06-06

I realized that there is a slight problem in the above calculations, since I did not consider that everything should be either nats or bits. In fact BC entropy is in base e, not base 2, and $s(2d)$ should be normalized in the above calculations. So the correct entropy in base 2 would be $\tilde{s}(x) := s(x) \times \log_2 e = \frac{x}{2} \log_2 e - \frac{x}{2} \log_2 x$. So $\tilde{s}(4) \approx -1.114$. Hence, the expected codeword length would be $m \log_2 n + \tilde{s}(2d) n$ which now becomes 4846060 bytes, which is close to the actual 4888812 bytes, but not as close as it seemed above. Difference per vertex = 0.042752 bytes. 

New test: $ n = 5 \times 10^6$, $m = 10000636$, $d = 2$, file name = `poi_5M.grp` edge size 10000636. Expected 27122010 compressed size: actual compressed file size: 27284623 bytes. Difference per vertex = 0.0325226 bytes. Compression timing ~5 min, decompression timing ~20 m. Compression ratio: ~21.8 bits / edge. 

<details>

<summary> Compression Timing </summary>

```
|---Init compressed () 2019-06-06 10:50:22 PM
|---Extact edge types () 2019-06-06 10:50:22 PM
|---|---Extract messages () 2019-06-06 10:50:22 PM
|---|---|---graph_message::update_message init () 2019-06-06 10:50:22 PM
|---|---|---resizing messages () 2019-06-06 10:50:22 PM
|---|---|---initializing messages () 2019-06-06 10:50:23 PM
|---|---|---updating messages () 2019-06-06 10:50:23 PM
|---|---|---* symmetrizing () 2019-06-06 10:50:23 PM
|---|---colored_graph::init init () 2019-06-06 10:50:24 PM
|---|---updating adj_list () 2019-06-06 10:50:25 PM
|---|---Find deg and ver_types () 2019-06-06 10:50:28 PM
|---Encode * vertices () 2019-06-06 10:50:30 PM
|---Encode * edges () 2019-06-06 10:51:37 PM
|---Encode vertex types () 2019-06-06 10:51:37 PM
|---Extract partition graphs () 2019-06-06 10:52:55 PM
|---Encode partition b graphs () 2019-06-06 10:53:00 PM
|---Encode partition graphs () 2019-06-06 10:53:00 PM
|---n () 2019-06-06 10:54:53 PM
|---h () 2019-06-06 10:54:53 PM
|---delta () 2019-06-06 10:54:53 PM
|---type_mark () 2019-06-06 10:54:53 PM
|---star_vertices () 2019-06-06 10:54:53 PM
|---star_edges () 2019-06-06 10:54:53 PM
|---vertex types () 2019-06-06 10:54:53 PM
|---partition bipartite graphs () 2019-06-06 10:54:53 PM
|---partition graphs () 2019-06-06 10:54:53 PM

|---Init compressed (): 0.000922s [0.000309%]
|---Extact edge types (): 7.967569s [2.670819%]
|---|---Extract messages (): 2.263304s [28.406450%]
|---|---|---graph_message::update_message init (): 0.055628s [2.457841%]
|---|---|---resizing messages (): 0.441868s [19.523134%]
|---|---|---initializing messages (): 0.795968s [35.168438%]
|---|---|---updating messages (): 0.000041s [0.001833%]
|---|---|---* symmetrizing (): 0.969267s [42.825321%]
|---|---colored_graph::init init (): 1.017033s [12.764660%]
|---|---updating adj_list (): 2.664784s [33.445381%]
|---|---Find deg and ver_types (): 2.022431s [25.383287%]
|---Encode * vertices (): 67.190575s [22.523037%]
|---Encode * edges (): 0.000030s [0.000010%]
|---Encode vertex types (): 77.823105s [26.087179%]
|---Extract partition graphs (): 4.915626s [1.647773%]
|---Encode partition b graphs (): 0.000037s [0.000012%]
|---Encode partition graphs (): 113.398972s [38.012611%]
|---n (): 0.000040s [0.000014%]
|---h (): 0.000007s [0.000002%]
|---delta (): 0.000006s [0.000002%]
|---type_mark (): 0.000007s [0.000002%]
|---star_vertices (): 0.000021s [0.000007%]
|---star_edges (): 0.000828s [0.000278%]
|---vertex types (): 0.002035s [0.000682%]
|---partition bipartite graphs (): 0.000040s [0.000013%]
|---partition graphs (): 6.490985s [2.175851%]

real	4m58.436s
user	4m47.772s
sys	0m5.596s
```

</details>

<details>

<summary> Decompression Timing </summary>

```
|---Init () 2019-06-06 10:59:11 PM
|---Decode * vertices () 2019-06-06 10:59:11 PM
|---Decode * edges () 2019-06-06 11:00:21 PM
|---Decode vertex types () 2019-06-06 11:00:21 PM
|---Decode partition graphs () 2019-06-06 11:02:12 PM
|---Decode partition b graphs () 2019-06-06 11:20:23 PM
|---Construct decoded graph () 2019-06-06 11:20:23 PM

|---Init (): 0.001214s [0.000092%]
|---Decode * vertices (): 70.394814s [5.362854%]
|---Decode * edges (): 0.000148s [0.000011%]
|---Decode vertex types (): 110.201263s [8.395408%]
|---Decode partition graphs (): 1092.062988s [83.196091%]
|---Decode partition b graphs (): 0.000035s [0.000003%]
|---Construct decoded graph (): 39.932926s [3.042190%]

real	21m52.152s
user	20m46.071s
sys	0m26.025s
```

</details>


### Experiment on time sensitivity on the number of edges. 

Fixed $n = 10000$, Poisson random graph with different values of $d$ and no marks. Fixed $h = 1$ and $d = 100$. All the files are in the folder `/Users/pdelgosha/Documents/Projects/Venkat/Codes/test/015_gcomp`


|      d |     number of edges | file name              | Compression time     |     time per node |     time per edge ($\mu$ s) |
| :----: | :-----------------: | :--------------------: | :------------------: | :---------------: | :-------------------------: |
|      2 |               19914 | `poi_10k_d_02.grp`     | 0.309 s              |              30.9 |                     15.5167 |
|      4 |               39775 | `poi_10k_d_04.grp`     | 0.457 s              |              45.7 |                     11.4896 |
|      8 |               79922 | `poi_10k_d_08.grp`     | 0.801 s              |              80.1 |                     10.0223 |
|     16 |              160448 | `poi_10k_d_16.grp`     | 1.859 s              |             185.9 |                     11.5863 |
|     32 |              318659 | `poi_10k_d_32.grp`     | 5.334 s              |             533.4 |                     16.7389 |

So it roughly increases semi linearly with the number of edges. 


## 2019-06-07 

I am trying to download and test the [EU-2005](http://law.di.unimi.it/webdata/eu-2005/) graph. I found [this](http://law.di.unimi.it/tutorial.php) page to learn how to use their graph format. First I need to make sure WebGraph is installed properly?

I installed Maven following the instructions in [this](https://maven.apache.org/install.html) link. Note that in Mac, the correct `bash_profile` analogous is `~/.profile` where you can add to PATH and all those stuff. 

I did the following which partly works: I downloaded the POM file from [here](https://search.maven.org/remotecontent?filepath=it/unimi/dsi/webgraph/3.6.1/webgraph-3.6.1.pom), changed its name to pom.xml, as well as the jar file from [here](https://search.maven.org/remotecontent?filepath=it/unimi/dsi/webgraph/3.6.1/webgraph-3.6.1.jar). Then, in the downloaded directory, entered `maven package`. It downloaded some packages and did some work, but there was a problem with one dependency: `Could not find artifact ch.qos.logback:logback-classic.jar:jar:3.6.1 in central`. 

<details>

<summary> details </summary>

```
[INFO] Scanning for projects...
[INFO]
[INFO] -----------------------< it.unimi.dsi:webgraph >------------------------
[INFO] Building WebGraph 3.6.1
[INFO] --------------------------------[ jar ]---------------------------------
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-resources-plugin/2.6/maven-resources-plugin-2.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-resources-plugin/2.6/maven-resources-plugin-2.6.pom (8.1 kB at 9.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/23/maven-plugins-23.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/23/maven-plugins-23.pom (9.2 kB at 307 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/22/maven-parent-22.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/22/maven-parent-22.pom (30 kB at 289 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/11/apache-11.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/11/apache-11.pom (15 kB at 529 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-resources-plugin/2.6/maven-resources-plugin-2.6.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-resources-plugin/2.6/maven-resources-plugin-2.6.jar (30 kB at 820 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-compiler-plugin/3.1/maven-compiler-plugin-3.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-compiler-plugin/3.1/maven-compiler-plugin-3.1.pom (10 kB at 464 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/24/maven-plugins-24.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/24/maven-plugins-24.pom (11 kB at 515 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/23/maven-parent-23.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/23/maven-parent-23.pom (33 kB at 795 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/13/apache-13.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/13/apache-13.pom (14 kB at 499 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-compiler-plugin/3.1/maven-compiler-plugin-3.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-compiler-plugin/3.1/maven-compiler-plugin-3.1.jar (43 kB at 1.5 MB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-surefire-plugin/2.12.4/maven-surefire-plugin-2.12.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-surefire-plugin/2.12.4/maven-surefire-plugin-2.12.4.pom (10 kB at 419 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire/2.12.4/surefire-2.12.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire/2.12.4/surefire-2.12.4.pom (14 kB at 445 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-surefire-plugin/2.12.4/maven-surefire-plugin-2.12.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-surefire-plugin/2.12.4/maven-surefire-plugin-2.12.4.jar (30 kB at 725 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-jar-plugin/2.4/maven-jar-plugin-2.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-jar-plugin/2.4/maven-jar-plugin-2.4.pom (5.8 kB at 307 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/22/maven-plugins-22.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/22/maven-plugins-22.pom (13 kB at 483 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/21/maven-parent-21.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/21/maven-parent-21.pom (26 kB at 599 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/10/apache-10.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/10/apache-10.pom (15 kB at 569 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-jar-plugin/2.4/maven-jar-plugin-2.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-jar-plugin/2.4/maven-jar-plugin-2.4.jar (34 kB at 851 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/ch/qos/logback/logback-classic.jar/3.6.1/logback-classic.jar-3.6.1.pom
[WARNING] The POM for ch.qos.logback:logback-classic.jar:jar:3.6.1 is missing, no dependency information available
Downloading from central: https://repo.maven.apache.org/maven2/it/unimi/dsi/fastutil/8.1.0/fastutil-8.1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/it/unimi/dsi/fastutil/8.1.0/fastutil-8.1.0.pom (1.4 kB at 3.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/it/unimi/dsi/sux4j/4.2.0/sux4j-4.2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/it/unimi/dsi/sux4j/4.2.0/sux4j-4.2.0.pom (3.3 kB at 38 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/it/unimi/dsi/dsiutils/2.4.2/dsiutils-2.4.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/it/unimi/dsi/dsiutils/2.4.2/dsiutils-2.4.2.pom (3.4 kB at 149 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/martiansoftware/jsap/2.1/jsap-2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/martiansoftware/jsap/2.1/jsap-2.1.pom (815 B at 48 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/guava/guava/23.2-jre/guava-23.2-jre.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/guava/guava/23.2-jre/guava-23.2-jre.pom (5.9 kB at 295 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/guava/guava-parent/23.2-jre/guava-parent-23.2-jre.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/guava/guava-parent/23.2-jre/guava-parent-23.2-jre.pom (9.1 kB at 286 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/oss/oss-parent/7/oss-parent-7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/oss/oss-parent/7/oss-parent-7.pom (4.8 kB at 284 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/code/findbugs/jsr305/1.3.9/jsr305-1.3.9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/code/findbugs/jsr305/1.3.9/jsr305-1.3.9.pom (965 B at 60 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/errorprone/error_prone_annotations/2.0.18/error_prone_annotations-2.0.18.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/errorprone/error_prone_annotations/2.0.18/error_prone_annotations-2.0.18.pom (1.6 kB at 91 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/errorprone/error_prone_parent/2.0.18/error_prone_parent-2.0.18.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/errorprone/error_prone_parent/2.0.18/error_prone_parent-2.0.18.pom (5.0 kB at 228 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/j2objc/j2objc-annotations/1.1/j2objc-annotations-1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/j2objc/j2objc-annotations/1.1/j2objc-annotations-1.1.pom (2.8 kB at 184 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/mojo/animal-sniffer-annotations/1.14/animal-sniffer-annotations-1.14.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/mojo/animal-sniffer-annotations/1.14/animal-sniffer-annotations-1.14.pom (2.5 kB at 121 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/mojo/animal-sniffer-parent/1.14/animal-sniffer-parent-1.14.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/mojo/animal-sniffer-parent/1.14/animal-sniffer-parent-1.14.pom (4.4 kB at 244 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/mojo/mojo-parent/34/mojo-parent-34.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/mojo/mojo-parent/34/mojo-parent-34.pom (24 kB at 553 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/codehaus-parent/4/codehaus-parent-4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/codehaus-parent/4/codehaus-parent-4.pom (4.8 kB at 284 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.7.25/slf4j-api-1.7.25.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.7.25/slf4j-api-1.7.25.pom (3.8 kB at 213 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-parent/1.7.25/slf4j-parent-1.7.25.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-parent/1.7.25/slf4j-parent-1.7.25.pom (14 kB at 643 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/ch/qos/logback/logback-classic/1.2.3/logback-classic-1.2.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/ch/qos/logback/logback-classic/1.2.3/logback-classic-1.2.3.pom (13 kB at 623 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/ch/qos/logback/logback-parent/1.2.3/logback-parent-1.2.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/ch/qos/logback/logback-parent/1.2.3/logback-parent-1.2.3.pom (18 kB at 481 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/ch/qos/logback/logback-core/1.2.3/logback-core-1.2.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/ch/qos/logback/logback-core/1.2.3/logback-core-1.2.3.pom (4.2 kB at 200 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-configuration/commons-configuration/1.10/commons-configuration-1.10.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-configuration/commons-configuration/1.10/commons-configuration-1.10.pom (21 kB at 575 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/32/commons-parent-32.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/32/commons-parent-32.pom (53 kB at 668 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-lang/commons-lang/2.6/commons-lang-2.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-lang/commons-lang/2.6/commons-lang-2.6.pom (17 kB at 729 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/17/commons-parent-17.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/17/commons-parent-17.pom (31 kB at 945 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/7/apache-7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/7/apache-7.pom (14 kB at 534 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.1.1/commons-logging-1.1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.1.1/commons-logging-1.1.1.pom (18 kB at 706 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/5/commons-parent-5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/5/commons-parent-5.pom (16 kB at 802 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/4/apache-4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/4/apache-4.pom (4.5 kB at 321 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.5/commons-io-2.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.5/commons-io-2.5.pom (13 kB at 492 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/39/commons-parent-39.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/39/commons-parent-39.pom (62 kB at 1.1 MB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/16/apache-16.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/16/apache-16.pom (15 kB at 855 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/20040616/commons-collections-20040616.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/20040616/commons-collections-20040616.pom (175 B at 13 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-math3/3.6.1/commons-math3-3.6.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-math3/3.6.1/commons-math3-3.6.1.pom (29 kB at 897 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.6/commons-io-2.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.6/commons-io-2.6.pom (14 kB at 679 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/42/commons-parent-42.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/42/commons-parent-42.pom (68 kB at 1.2 MB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/18/apache-18.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/18/apache-18.pom (16 kB at 712 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-api/2.1.1/jung-api-2.1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-api/2.1.1/jung-api-2.1.1.pom (1.2 kB at 72 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-parent/2.1.1/jung-parent-2.1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-parent/2.1.1/jung-parent-2.1.1.pom (9.6 kB at 419 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/guava/guava/19.0/guava-19.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/guava/guava/19.0/guava-19.0.pom (6.8 kB at 377 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/guava/guava-parent/19.0/guava-parent-19.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/guava/guava-parent/19.0/guava-parent-19.0.pom (9.9 kB at 494 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-io/2.1.1/jung-io-2.1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-io/2.1.1/jung-io-2.1.1.pom (1.2 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-algorithms/2.1.1/jung-algorithms-2.1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-algorithms/2.1.1/jung-algorithms-2.1.1.pom (1.1 kB at 63 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/junit/junit/4.12/junit-4.12.pom
Downloaded from central: https://repo.maven.apache.org/maven2/junit/junit/4.12/junit-4.12.pom (24 kB at 764 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/hamcrest/hamcrest-core/1.3/hamcrest-core-1.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/hamcrest/hamcrest-core/1.3/hamcrest-core-1.3.pom (766 B at 59 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/hamcrest/hamcrest-parent/1.3/hamcrest-parent-1.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/hamcrest/hamcrest-parent/1.3/hamcrest-parent-1.3.pom (2.0 kB at 164 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/emma/emma/2.1.5320/emma-2.1.5320.pom
Downloaded from central: https://repo.maven.apache.org/maven2/emma/emma/2.1.5320/emma-2.1.5320.pom (1.3 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/emma/emma_ant/2.1.5320/emma_ant-2.1.5320.pom
Downloaded from central: https://repo.maven.apache.org/maven2/emma/emma_ant/2.1.5320/emma_ant-2.1.5320.pom (1.3 kB at 75 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-lang3/3.7/commons-lang3-3.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-lang3/3.7/commons-lang3-3.7.pom (28 kB at 787 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/ch/qos/logback/logback-classic.jar/3.6.1/logback-classic.jar-3.6.1.jar
Downloading from central: https://repo.maven.apache.org/maven2/it/unimi/dsi/fastutil/8.1.0/fastutil-8.1.0.jar
Downloading from central: https://repo.maven.apache.org/maven2/it/unimi/dsi/sux4j/4.2.0/sux4j-4.2.0.jar
Downloading from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.6/commons-io-2.6.jar
Downloading from central: https://repo.maven.apache.org/maven2/commons-lang/commons-lang/2.6/commons-lang-2.6.jar
Downloading from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/20040616/commons-collections-20040616.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.6/commons-io-2.6.jar (215 kB at 382 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/it/unimi/dsi/dsiutils/2.4.2/dsiutils-2.4.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-lang/commons-lang/2.6/commons-lang-2.6.jar (284 kB at 364 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/guava/guava/23.2-jre/guava-23.2-jre.jar
Downloaded from central: https://repo.maven.apache.org/maven2/it/unimi/dsi/sux4j/4.2.0/sux4j-4.2.0.jar (394 kB at 428 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/code/findbugs/jsr305/1.3.9/jsr305-1.3.9.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/code/findbugs/jsr305/1.3.9/jsr305-1.3.9.jar (33 kB at 35 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/errorprone/error_prone_annotations/2.0.18/error_prone_annotations-2.0.18.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/errorprone/error_prone_annotations/2.0.18/error_prone_annotations-2.0.18.jar (12 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/j2objc/j2objc-annotations/1.1/j2objc-annotations-1.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/20040616/commons-collections-20040616.jar (563 kB at 551 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/mojo/animal-sniffer-annotations/1.14/animal-sniffer-annotations-1.14.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/j2objc/j2objc-annotations/1.1/j2objc-annotations-1.1.jar (8.8 kB at 8.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.7.25/slf4j-api-1.7.25.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/mojo/animal-sniffer-annotations/1.14/animal-sniffer-annotations-1.14.jar (3.5 kB at 3.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-math3/3.6.1/commons-math3-3.6.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.7.25/slf4j-api-1.7.25.jar (41 kB at 37 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-api/2.1.1/jung-api-2.1.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-api/2.1.1/jung-api-2.1.1.jar (42 kB at 33 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-io/2.1.1/jung-io-2.1.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/it/unimi/dsi/dsiutils/2.4.2/dsiutils-2.4.2.jar (439 kB at 339 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-algorithms/2.1.1/jung-algorithms-2.1.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-io/2.1.1/jung-io-2.1.1.jar (80 kB at 49 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/martiansoftware/jsap/2.1/jsap-2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/martiansoftware/jsap/2.1/jsap-2.1.jar (69 kB at 38 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/junit/junit/4.12/junit-4.12.jar
Downloaded from central: https://repo.maven.apache.org/maven2/net/sf/jung/jung-algorithms/2.1.1/jung-algorithms-2.1.1.jar (230 kB at 123 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/hamcrest/hamcrest-core/1.3/hamcrest-core-1.3.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/hamcrest/hamcrest-core/1.3/hamcrest-core-1.3.jar (45 kB at 22 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/emma/emma/2.1.5320/emma-2.1.5320.jar
Downloaded from central: https://repo.maven.apache.org/maven2/junit/junit/4.12/junit-4.12.jar (315 kB at 132 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/emma/emma_ant/2.1.5320/emma_ant-2.1.5320.jar
Downloaded from central: https://repo.maven.apache.org/maven2/emma/emma_ant/2.1.5320/emma_ant-2.1.5320.jar (41 kB at 15 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/ch/qos/logback/logback-classic/1.2.3/logback-classic-1.2.3.jar
Downloaded from central: https://repo.maven.apache.org/maven2/ch/qos/logback/logback-classic/1.2.3/logback-classic-1.2.3.jar (290 kB at 88 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/emma/emma/2.1.5320/emma-2.1.5320.jar (468 kB at 142 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-configuration/commons-configuration/1.10/commons-configuration-1.10.jar
Downloading from central: https://repo.maven.apache.org/maven2/ch/qos/logback/logback-core/1.2.3/logback-core-1.2.3.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-configuration/commons-configuration/1.10/commons-configuration-1.10.jar (363 kB at 92 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.1.1/commons-logging-1.1.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.1.1/commons-logging-1.1.1.jar (61 kB at 15 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-lang3/3.7/commons-lang3-3.7.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-math3/3.6.1/commons-math3-3.6.1.jar (2.2 MB at 535 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/ch/qos/logback/logback-core/1.2.3/logback-core-1.2.3.jar (472 kB at 113 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/guava/guava/23.2-jre/guava-23.2-jre.jar (2.6 MB at 625 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-lang3/3.7/commons-lang3-3.7.jar (500 kB at 115 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/it/unimi/dsi/fastutil/8.1.0/fastutil-8.1.0.jar (19 MB at 1.9 MB/s)
[INFO] ------------------------------------------------------------------------
[INFO] BUILD FAILURE
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  14.646 s
[INFO] Finished at: 2019-06-07T16:02:00-07:00
[INFO] ------------------------------------------------------------------------
[ERROR] Failed to execute goal on project webgraph: Could not resolve dependencies for project it.unimi.dsi:webgraph:jar:3.6.1: Could not find artifact ch.qos.logback:logback-classic.jar:jar:3.6.1 in central (https://repo.maven.apache.org/maven2) -> [Help 1]
[ERROR]
[ERROR] To see the full stack trace of the errors, re-run Maven with the -e switch.
[ERROR] Re-run Maven using the -X switch to enable full debug logging.
[ERROR]
[ERROR] For more information about the errors and possible solutions, please read the following articles:
[ERROR] [Help 1] http://cwiki.apache.org/confluence/display/MAVEN/DependencyResolutionException
```

</details>


I am confused and tired, will continue later!

## 2019-06-10

I tried to install `ch.qos.logback` separately from [here](https://mvnrepository.com/artifact/ch.qos.logback/logback-classic/1.2.3) but it did not help. What I did was to download both jar and pom files in a directory, and run `mvn install:install-file -Dfile=logback-classic-1.2.3.jar` (I found this from [here](https://maven.apache.org/guides/mini/guide-3rd-party-jars-local.html)). It run with no error, but did not help with webgraph, it still gives logback errors. 

Finally, I removed the lines related to logback from the pom file and it built with no errors. However, I am not sure if it works properly. Going to test it. 

Not sure how to test it, trying to follow the instructions in [this](http://law.di.unimi.it/tutorial.php) page to first download [eu-2005](http://law.di.unimi.it/webdata/eu-2005/). First run `for ext in .properties .graph .md5sums; do wget -c http://data.law.di.unimi.it/webdata/eu-2005/eu-2005$ext; done`. Did not check md5sum for now. Run `java it.unimi.dsi.webgraph.BVGraph -o -O -L eu-2005`, but I get the error 

```
Error: Could not find or load main class it.unimi.dsi.webgraph.BVGraph
Caused by: java.lang.ClassNotFoundException: it.unimi.dsi.webgraph.BVGraph
```

I do not know what to do. Even tried to install webgraph++, but got boost errors. I came across [this](https://groups.google.com/forum/#!topic/web-data-commons/v4n3KdPPSWU) google group page but not sure if it is helpful. 


## 2019-06-12

I realized that the boost header files that webgraph++ needs exist on my machine, but perhaps it can not find it. They are located on `/opt/local/include/boost/`. This can be given via `-I` command to `gcc`, but I do not know how to change their `Makefile` to force this. Trying to see what is the default place to find include files. I realized from [here](https://stackoverflow.com/questions/344317/where-does-gcc-look-for-c-and-c-header-files) answer by user292283 that the set of paths the compiler looks for can be obtained by `cpp -v`. I tried 

```
sudo cp -R /opt/local/include/boost/ /usr/local/include/boost
```


to see if now the program can see boost headers. With this, I receive other errors, which seems to be related to their code, not compiling:

~~~~~~
/Applications/Xcode.app/Contents/Developer/usr/bin/make -C asciigraph all
g++ -I/u/jpr/old_code/webgraph -L/u/jpr/old_code/webgraph -Wall -DCONFIG_FAST -O3  -c offline_edge_iterator.cpp
clang: warning: argument unused during compilation: '-L/u/jpr/old_code/webgraph'
g++ -I/u/jpr/old_code/webgraph -L/u/jpr/old_code/webgraph -Wall -DCONFIG_FAST -O3  -c offline_vertex_iterator.cpp
clang: warning: argument unused during compilation: '-L/u/jpr/old_code/webgraph'
g++ -I/u/jpr/old_code/webgraph -L/u/jpr/old_code/webgraph -Wall -DCONFIG_FAST -O3  -c offline_graph.cpp
clang: warning: argument unused during compilation: '-L/u/jpr/old_code/webgraph'
offline_graph.cpp:70:4: error: use of undeclared identifier 'tie'; did you mean 'boost::tie'?
   tie( b, e ) = result.get_edge_iterator();
   ^~~
   boost::tie
/usr/local/include/boost/tuple/detail/tuple_basic.hpp:877:1: note: 'boost::tie' declared here
tie(T0& t0, T1& t1) {
^
1 error generated.
make[1]: *** [offline_graph.o] Error 1
make: *** [libs] Error 2
~~~~~~

I am thinking that it might be the case that the pom file is only meant to install the dependencies, not necessarily the package itself? The reason is that in directory `~/.m2/repository/it/unim/dsi`, I can see folders with names `dsiutils`, `fastutil` and `sux4j`, which  surprisingly appear in the [webgraph maven page](https://search.maven.org/search?q=g:it.unimi.dsi) as well. But there is no `webgraph` page. So I created a folder named `webgraph`, inside it a folder named `3.6.1`, and I copied `webgraph-3.6.1.jar`. Lets see what happens.  Again I receive the same error that the class not found. 


## 2019-06-13 

I came across [this](https://stackoverflow.com/questions/21918454/trying-to-install-webgraph-to-run-a-class) stackoverflow page which seems to be quite relevant, because I get the same error. I realized that I do not know how to install the jar file, but rather I can use `java -cp` to specify the __class path__ which is basically the address of the desired jar file. So I copied webgraph jar file near the eu-2005 files, and tried to run `java -cp ./webgraph-3.6.1.jar it.unimi.dsi.webgraph.BVGraph -o -O -L eu-2005`, but I got the error 

```
Error: Could not find or load main class it.unimi.dsi.webgraph.BVGraph
Caused by: java.lang.NoClassDefFoundError: it/unimi/dsi/lang/FlyweightPrototype
```

Which is very close to the one mentioned in the stackoverflow page above. 
