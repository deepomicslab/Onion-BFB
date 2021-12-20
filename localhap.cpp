#include <iostream>
#include <fstream>
#include <cstring>
#include <set>
#include <map>
#include <string>
#include <stack>

#include "Graph.hpp"
#include "Exceptions.hpp"
#include "LocalGenomicMap.hpp"
#include "JunctionDB.hpp"
#include "cxxopts.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    cxxopts::Options options("localhap", "Local Haplotype constructer");

    options.add_options()
            ("op", "operate: check or solve", cxxopts::value<std::string>())
            ("juncdb", "Junction database", cxxopts::value<std::string>())
            ("in_lh", "Input lh file", cxxopts::value<std::string>())
            ("out_lh", "Checked local hap input file, lh format", cxxopts::value<std::string>())
            ("lp_prefix", "ILP out file prefix, only for check", cxxopts::value<std::string>())
            ("verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))
            ("hic_matrix", "Segment Hic matrix file, only for solve", cxxopts::value<std::string>())
            ("tgs_order", "Segment tgs local order file, only for solve", cxxopts::value<std::string>())
            ("hap", "Haplotype out file, only for solve", cxxopts::value<std::string>())
            ("traversed", "traversed path out file, only for solve", cxxopts::value<std::string>())
            ("circuits", "Circuits out file, only for solve", cxxopts::value<std::string>())
            ("help", "Print usage");
    auto result = options.parse(argc, argv);
    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    result["op"].as<std::string>().c_str();
    std::cout << result["op"].as<std::string>().c_str() << std::endl;

    if (strcmp(result["op"].as<std::string>().c_str(), "check") == 0) {
        const char *juncdbFn_sample = result["juncdb"].as<std::string>().c_str();
        const char *lhRawFn = result["in_lh"].as<std::string>().c_str();
        const char *lhCheckedFn = result["out_lh"].as<std::string>().c_str();
        const char *lpFn = result["lp_prefix"].as<std::string>().c_str();
        bool verbose = result["verbose"].as<bool>();
        try {
            auto *db_sample = new JunctionDB(juncdbFn_sample);
            db_sample->sortRecordEntry();
//            db_sample->print();

            Graph *g = new Graph(lhRawFn);
            g->calculateHapDepth();
            g->checkLowerBound();
            g->print();
            LocalGenomicMap *lgm = new LocalGenomicMap(g);
            cout << "checking reachability" << endl;
            g->print();
            lgm->checkReachability(db_sample, verbose);

            g->calculateCopyNum();
            g->checkOrphan();
            g->print();

            // lgm->checkInferredJunctionCredibility();
            cout << "connect ss" << endl;
            try {
                lgm->connectSourceSink();
            } catch (DuplicateJunctionException &e) {
                cout << e.what() << endl;
            }
//            g->calculateCopyNum();
            g->print();
            cout << "write" << endl;
            g->writeGraph(lhCheckedFn);
            cout << "write done" << endl;

            int feasible = lgm->balancerILP(lpFn);

            cout << "Done" << endl;
        } catch (DuplicateJunctionException &e) {
            cout << e.what() << endl;
            return 1;
        } catch (SegmentDoesNotExistException &e) {
            cout << e.what() << endl;
            return 1;
        } catch (JunctionDoesNotExistException &e) {
            cout << e.what() << endl;
            return 1;
        } catch (BackwardReachSourceNegativeException &e) {
            cout << e.what() << endl;
            return 1;
        } catch (BackwardReachSinkPositiveException &e) {
            cout << e.what() << endl;
            return 1;
        } catch (ForwardReachSinkNegativeException &e) {
            cout << e.what() << endl;
            return 1;
        } catch (ForwardReachSourcePositiveException &e) {
            cout << e.what() << endl;
            return 1;
            // } catch (...) {
            //     cout << "Logic error." << endl;
            //     return 1 ;
        }
        return 0;
    } else if (strcmp(result["op"].as<std::string>().c_str(), "solve") == 0) {
        const char *juncdbFn = result["juncdb"].as<std::string>().c_str();
        const char *lhFn = result["in_lh"].as<std::string>().c_str();
        // const char * lpOutFn = argv[4];
        const char *circuitsFn = result["circuits"].as<std::string>().c_str();
        const char *traversedFn = result["traversed"].as<std::string>().c_str();
        const char *hapFn = result["hap"].as<std::string>().c_str();
        bool verbose = result["verbose"].as<bool>();
        const char *longFragFn;
        const char *hicMatrix;
        try {
            JunctionDB *db = new JunctionDB(juncdbFn);
            db->sortRecordEntry();
            // db->writeDB("/home/tanbowen/LocalHap/test/test.db");
            // db->print();

            Graph *g = new Graph(lhFn);
            // g->checkOrphan();
            // g->print();
            // g->calculateCopyNum();
            // g->checkLowerBound();
            // g->print();
            // g->calculateCopyNum();
            g->print();
            LocalGenomicMap *lgm = new LocalGenomicMap(g);
            if (result.count("tgs_order")) {
                longFragFn = result["tgs_order"].as<std::string>().c_str();
                lgm->read_long_frags(longFragFn);
                lgm->setUsingLong(true);
                for (auto frag : *lgm->get_long_frags()) {
                    for(VertexPath* suvp : *frag.second) {
                        for (Vertex *v: *suvp) {
                            cout << v->getInfo() << " ";
                        }
                    }
                    cout << endl;
                }
            }
            if (result.count("hic_matrix")) {
                hicMatrix = result["hic_matrix"].as<std::string>().c_str();

                lgm->read_hic_matrix(hicMatrix);
            }
//             lgm->read_hic_matrix(hicMatrix);
//             return 0;
            // TODO some problems on checking reachability
            // especially with inversion
            // cout << "add normal" << endl;
            // lgm->addNormalJunctions();
            // g->print();
            // lgm->addNormalJunctions();
            // lgm->checkReachability(db, verbose);
            // lgm->addAllJuncsFromDB(db);
            // g->checkOrphan();
            // g->print();

            // lgm->checkInferredJunctionCredibility();
            // g->print();

            /* integer linear programming */
            try {
//                lgm->connectSourceSink();
            } catch (DuplicateJunctionException &e) {
                cout << e.what() << endl;
            }
            g->print();
            // int feasible = lgm->balancerILP(lpOutFn);
            // if (feasible >= 0) cout << "feasible" << endl;
            // if (feasible < 0) return 1;
            // cout << "After balancing: >>>>>>>>>>>>" << endl;
            // g->print();

            // delete g->getJunctions()->back();
            // g->getJunctions()->pop_back();
            // g->print();

            cout << "Identifying circuits..." << endl;
            lgm->traverseGraph(db);
            // g->print();
            // lgm->printCircuits();
            // cout << "Extracting circuits..." << endl;
            lgm->writeTraversedPath(traversedFn);
            lgm->extractCircuits();
            lgm->sortCircuits();
            lgm->divideCircuits();
            // g->print();
            if (verbose) {
               lgm->printCircuits();
            }

           lgm->writeCircuits(circuitsFn);

            cout << "Generating haploids..." << endl;
            g->print();
//            lgm->generateHaploids();
            // // cout << "Estimated haploids: " << endl;
            // // lgm->printHaploids();
//            lgm->writeHaploids(hapFn);
            // cout << "Done" << endl;
        } catch (DuplicateJunctionException &e) {
            cout << e.what() << endl;
            return 1;
        } catch (SegmentDoesNotExistException &e) {
            cout << e.what() << endl;
            return 1;
        } catch (JunctionDoesNotExistException &e) {
            cout << e.what() << endl;
            return 1;
        } catch (ILPBalancerInfeasibleException &e) {
            cout << e.what() << endl;
            return 1;
        } catch (BackwardReachSourceNegativeException &e) {
            cout << e.what() << endl;
            return 1;
        } catch (BackwardReachSinkPositiveException &e) {
            cout << e.what() << endl;
            return 1;
        } catch (ForwardReachSinkNegativeException &e) {
            cout << e.what() << endl;
            return 1;
        } catch (ForwardReachSourcePositiveException &e) {
            cout << e.what() << endl;
            return 1;
            // } catch (...) {
            //     cout << "Logic error." << endl;
            //     return 1 ;
        }
    }else if (strcmp(result["op"].as<std::string>().c_str(), "bfb") == 0) {
        const char *lhRawFn = result["in_lh"].as<std::string>().c_str();
        const char *lpFn = result["lp_prefix"].as<std::string>().c_str();
        Graph *g = new Graph(lhRawFn);
        // g->calculateHapDepth();
        // g->calculateCopyNum();
        LocalGenomicMap *lgm = new LocalGenomicMap(g);
        vector<Segment *> sources = *g->getMSources();
        vector<Segment *> sinks = *g->getMSinks();

        //output copy numbers of junctions
        // string sample = result["lp_prefix"].as<std::string>();
        // stringstream juncInfo;
        // for(Junction* junc: *g->getJunctions()) {
        //     juncInfo << sample << "\t"<<junc->getSource()->getId()<<":"<<junc->getSourceDir()<<":"
        //         <<junc->getTarget()->getId()<<":"<<junc->getTargetDir()<<"\t"<<junc->getWeight()->getCopyNum()<<"\n";            
        // }
        // ofstream juncFile;
        // juncFile.open("COLO829_juncCN.txt",std::ios_base::app);
        // juncFile<<juncInfo.str();
        // juncFile.close();
        // exit(0);

        vector<vector<int>> bfbPaths;
        //construct bfb path on each chromosome
        for (int n=0; n<g->getMSources()->size(); n++) {
            //enumerate all the patterns and loops
            int startID = sources[n]->getId();
            int endID = sinks[n]->getId();
            vector<vector<int>> patterns, loops;
            vector<int> temp;
            lgm->combinations(startID,endID,2,patterns,temp);
            temp.clear();
            lgm->combinations(startID,endID,2,loops,temp);

            //construct mapping from pattern/loop to index
            map<string, int> variableIdx;
            for (int i=0;i<patterns.size();i++) {
                string key = "p:"+to_string(patterns[i][0])+","+to_string(patterns[i][1]);
                variableIdx[key] = i;
                cout<<variableIdx[key]<<" "<<key<<endl;
            }
            for (int i=0;i<loops.size();i++) {
                string key = "l:"+to_string(loops[i][0])+","+to_string(loops[i][1]);
                variableIdx[key] = i+patterns.size();
                cout<<variableIdx[key]<<" "<<key<<endl;
            }

            //find copy number for both normal junctions and fold-back inversions
            vector<Junction *> inversions;
            double** juncCN = new double*[endID+1];
            for (int i=0; i <= endID; i++) {
                juncCN[i] = new double[2];//0: normal junction   1: inversion
                memset(juncCN[i], 0, 2*sizeof(double));
            }
            for (Junction *junc: *(g->getJunctions())) {
                int sourceID = junc->getSource()->getId(), targetID = junc->getTarget()->getId();
                char sourceDir = junc->getSourceDir(), targetDir = junc->getTargetDir();
                if (sourceID<startID||sourceID>endID||targetID<startID||targetID>endID) continue;
                double copyNum = junc->getWeight()->getCopyNum();
                if (0.5 < copyNum && copyNum < 1)
                    copyNum = 1;//round small CN to 1
                if (sourceDir == '+' && targetDir == '+') {//ht or th: not consider deletion on the chromosome
                    if (sourceID+1 == targetID) {//normal edges                
                        juncCN[sourceID][0] += copyNum;
                    }
                    else if (sourceID-1 == targetID) {//normal edges (negative strand)
                        juncCN[targetID][0] += copyNum;
                    }
                }
                else {//hh or tt (inversion)
                    if (abs(sourceID-targetID)<=3) {//fold-back inversion (with error of 3 bp)
                        if(sourceID != targetID)
                            inversions.push_back(junc);
                        if (sourceDir == '+') {
                            int greaterID = sourceID>targetID? sourceID:targetID;
                            juncCN[greaterID][1] += copyNum;
                        }
                        else {
                            int smallerID = sourceID<targetID? sourceID:targetID;
                            juncCN[smallerID][1] += copyNum;
                        }                        
                    }
                }            
            }
            //check if there is any fold-back inversion
            cout<<"Junction CN"<<endl;
            double inversionCNSum = 0;
            for (int i=0;i<=endID;i++) {
                inversionCNSum += juncCN[i][1];
                cout<<i<<","<<i+1<<" "<<juncCN[i][0]<<"\t"<<i<<","<<i<<" "<<juncCN[i][1]<<endl;
            }
            //copy number of patterns and loops
            int* elementCN = new int[variableIdx.size()];
            memset(elementCN, 0, variableIdx.size()*sizeof(int));
            if (abs(inversionCNSum)<0.000001) {//no fold-back inversion
                vector<int> temp({startID, endID, endID, startID});
                lgm->editInversions(temp, inversions, juncCN, elementCN, variableIdx);
                bfbPaths.push_back(temp);
                continue;
            }
            
            //construct ILP and generate .lp file for cbc
            lgm->BFB_ILP(lpFn, patterns, loops, variableIdx, juncCN);

            //run cbc under the directory containing test.lp
            string str = "cbc "+string(lpFn) +".lp solve solu "+string(lpFn)+".sol";
            const char *cmd = str.c_str();
            system(cmd);
            //read patterns and loops from .sol
            str = "./" + string(lpFn)+".sol";
            const char *solDir = str.c_str();
            ifstream solFile(solDir);
            if (!solFile) {
                cerr << "Cannot open file " << solDir << endl;
                exit(1);
            }            
            string element, cn;
            while (solFile >> element) {
                if (element[0] == 'x') {                
                    int idx = stoi(element.substr(1));
                    if (idx < variableIdx.size()) {//exclude epsilons
                        solFile >> cn;
                        int copynum = stoi(cn);
                        elementCN[idx] = copynum;
                    }
                }
            }
            //construct BFB DAG and find all topological orders
            vector<vector<int>> adj, node2pat, node2loop;
            lgm->constructDAG(adj, node2pat, node2loop, variableIdx, elementCN);
            int num = adj.size();
            bool *visited = new bool[num];
            int *indeg = new int[num];
            for (int i = 0; i < num; i++) {
                visited[i] = false;
                indeg[i] = 0;
            }
            //set up indegree
            int cnt = 0;
            for (int i = 0; i < num; i++) {
                for (auto next = adj[i].begin(); next != adj[i].end(); next++) {
                    indeg[*next]++;                
                }
                cout<<i+1<<": ";
                for (int j=0;j<adj[i].size();j++) {
                    cout<<adj[i][j]+1<<" ";
                }
                cout<<endl;
            }
            //find all topological orders in BFB DAG
            vector<int> res;
            vector<vector<int>> orders;
            lgm->allTopologicalOrders(res, visited, num, indeg, adj, orders);
            cout<<"All topological orders: "<<endl;
            for (vector<int> bfb: orders) {
                for (int i=0;i<bfb.size();i++)
                    cout<<bfb[i]+1<<" ";
                cout<<endl;
            }
            //get one valid bfb path
            vector<int> path;
            lgm->getBFB(orders, node2pat, node2loop, path);//get a valid BFB path
            //output the text for visualization
            lgm->editInversions(path, inversions, juncCN, elementCN, variableIdx);//edit the imperfect fold-back inversions (with deletion)
            bfbPaths.push_back(path);
        }        
        //print the result
        // for (int i=0;i<bfbPaths.size();i++)
        //     lgm->printBFB(bfbPaths[i]);

        //parameters for dealing with other SVs
        string mainChr;
        vector<string> insChr, conChr;
        vector<int> startSegs;//starting segments for insertions
        lgm->readBFBProps(mainChr, insChr, conChr, startSegs, lhRawFn);//read properties        
        
        //find other SVs based on graph of segments
        vector<Junction *> insertionSV, concatenationSV;
        vector<Segment *> segments = *g->getSegments();
        int segNum = segments.size()+1;
        int** connections = new int*[segNum];
        for (int i=0; i < segNum; i++) {
            connections[i] = new int[segNum];
            memset(connections[i], -1, segNum*sizeof(int));
        }
        //find a range for normal links of each chromosome
        vector<int> maxSeg, minSeg;
        for (int i=0; i<g->getMSources()->size(); i++) {
            maxSeg.push_back((*g->getMSources())[i]->getId());
            minSeg.push_back((*g->getMSinks())[i]->getId());
        }
        for (Junction *junc: *g->getJunctions()) {
            if (junc->isInferred())
                continue;
            Segment *source = junc->getSource();
            Segment *target = junc->getTarget();
            int chr1 = source->getChrId(), chr2 = target->getChrId();
            if (chr1 != chr2) {
                int s = source->getId(), e = target->getId();
                if (s > maxSeg[chr1]) maxSeg[chr1] = s;
                if (s < minSeg[chr1]) minSeg[chr1] = s;
                if (e > maxSeg[chr2]) maxSeg[chr2] = e;
                if (e < minSeg[chr2]) minSeg[chr2] = e;
            }
        }        
        //get all SVs for insertion
        for (Junction *junc: *g->getJunctions()) {
            if (junc->isInferred())
                continue;
            Segment *source = junc->getSource();
            Segment *target = junc->getTarget();
            int chr1 = source->getChrId(), chr2 = target->getChrId();//index for chromosome
            string chrNum1 = source->getChrom(), chrNum2 = target->getChrom();//chromosome name
            int sourceId = source->getId(), targetId = target->getId();
            //sv for concatenation
            if (chr1 != chr2 && find(conChr.begin(),conChr.end(),chrNum1)!=conChr.end() &&
                find(conChr.begin(),conChr.end(),chrNum2)!=conChr.end()) {
                concatenationSV.push_back(junc);             
            }
            //sv for insertion
            if (find(insChr.begin(),insChr.end(),chrNum1)!=insChr.end() &&
                find(insChr.begin(),insChr.end(),chrNum2)!=insChr.end()) {
                if (chr1 != chr2) {
                    insertionSV.push_back(junc);
                    connections[sourceId][targetId] = insertionSV.size()-1;
                    connections[targetId][sourceId] = insertionSV.size()-1;
                }
                else if (chrNum1 != mainChr) {//chr1==chr2: normal junctions for insertion
                    if ((minSeg[chr1] <= sourceId && sourceId <= maxSeg[chr1]) &&
                        (minSeg[chr1] <= targetId && targetId <= maxSeg[chr1])) {
                        insertionSV.push_back(junc);
                        connections[sourceId][targetId] = insertionSV.size()-1;
                        connections[targetId][sourceId] = insertionSV.size()-1;
                    }
                }
            }
        }
        cout<<"sv for concatenation: "<<endl;
        for (Junction *junc: concatenationSV) {
            cout<<junc->getSource()->getId()<<" "<<junc->getTarget()->getId()<<endl;
        }
        cout<<"sv for insertion: "<<endl;
        for (Junction *junc: insertionSV) {
            cout<<junc->getSource()->getId()<<" "<<junc->getTarget()->getId()<<endl;
        }
        
        //construct bfb paths with insertion        
        for (int i=0; i<startSegs.size(); i++) {
            if (insertionSV.size() == 0) break;
            //find a path by traversing all the SVs with DFS
            vector<int> segs;//segments in sequence
            bool finished = false;
            bool *visited = new bool[segNum];
            memset(visited, false, segNum*sizeof(bool));
            stack<int> s;
            int startSeg = startSegs[i];
            int startChr = segments[startSeg-1]->getChrId();
            s.push(startSeg);//starting segment
            visited[startSeg] = true;
            while (!s.empty()) {
                int front = s.top();
                // cout<<front<<" "<<segments[front-1]->getChrId()<<endl;
                segs.push_back(front);
                s.pop();
                //adjacent segments
                for (int next=segNum-1; next>=1; next--) {
                    if (!visited[next] && connections[front][next] != -1) {
                        s.push(next);
                        visited[next] = true;
                        if (segments[next-1]->getChrId() == startChr) {
                            segs.push_back(next);
                            finished = true;
                            break;
                        }
                    }
                }
                if (finished)
                    break;
            }
            if (segments[segs.back()-1]->getChrId() != startChr)
                segs.push_back(startSeg);
            cout<<"The sequence of segments"<<endl;
            for (int idx: segs)
                cout<<idx<<" ";
            cout<<endl;

            //construct the bool array for SV directions
            bool *edgeA = new bool[segs.size()];
            memset(edgeA, true, segs.size()*sizeof(bool));
            vector<Junction *> svPath;
            //find valid SVs in sequence
            int cnt = 0;
            for (int i=0; i<segs.size()-1; i+=1) {
                if(connections[segs[i]][segs[i+1]] == -1)
                    break;
                Junction *sv = insertionSV[connections[segs[i]][segs[i+1]]];
                Edge *e = sv->getEdgeA();
                int chr1 = sv->getSource()->getChrId(), chr2 = sv->getTarget()->getChrId();
                //cout<<chr1<<" "<<chr2<<" "<<endl;
                if (chr1 == chr2) continue;
                svPath.push_back(sv);
                int sourceId = e->getSource()->getId(), targetId = e->getTarget()->getId();                
                if (sourceId == segs[i+1] && targetId == segs[i])
                    edgeA[cnt] = false;
                cnt++;
            }
            //print bfb path with insertions
            if(svPath.empty())
                continue;
            cout<<"bfb path with insertions: "<<i<<endl;
            vector<int> res;
            lgm->bfbInsertion(svPath, bfbPaths, edgeA, res);
            // for (int n: res)
            //     cout<<n<<" ";
            // cout<<endl;
            vector<int> output;
            lgm->editBFB(bfbPaths, res, output);
            //lgm->printBFB(output);
        }
        //construct bfb paths with concatenation        
        for (int i=0; i<concatenationSV.size(); i++) {
            cout<<"bfb paths with concatenation: "<<i<<endl;
            vector<int> res;
            lgm->bfbConcate(concatenationSV[i], true, 2, 0, bfbPaths, res);//start from position 2 of the main chromosome
            if (!res.empty()) {
                vector<int> output;
                lgm->editBFB(bfbPaths, res, output);
                //lgm->printBFB(output);
            }         
        }

    } else if (strcmp(result["op"].as<std::string>().c_str(), "bpm") == 0) {
        const char *lhRawFn = result["in_lh"].as<std::string>().c_str();
        Graph *g = new Graph(lhRawFn);
        LocalGenomicMap *lgm = new LocalGenomicMap(g);
        //Extract SV and normal edges and divide segments into two parts
        vector<Junction *> sv, normal;
        vector<Segment *> sources, targets;
        int num =  g->getSegments()->size();
        for (Junction *junc: *(g->getJunctions())){
            if (junc->getSource()->getId()%num +1  == junc->getTarget()->getId() &&
                junc->getSourceDir()=='+' && junc->getTargetDir()=='+') {                
                normal.push_back(junc);
            }
            else {
                sv.push_back(junc);
            }                
        }
        //Find the maximum bipartite matching
        vector<Junction *> selectedJunc;
        lgm->findMaxBPMatching(sv, selectedJunc);
        // for (Junction *junc: selectedJunc) {
        //     cout<<junc->getSource()->getId()<<" -> "<<junc->getTarget()->getId()<<endl;
        // }

        //Find the Eulerian Circuit with the selected and normal junctions
        vector<vector<int>> adj; //adjacency list
        for (int i=0; i<=num; i++) {
            vector<int> temp;
            adj.push_back(temp);
        }
        cout<<"SV: "<<endl;
        for (Junction* junc: selectedJunc) {
            cout<<junc->getSource()->getId()<<"->"<<junc->getTarget()->getId()<<endl;
            adj[junc->getSource()->getId()].push_back(junc->getTarget()->getId());
        }
        cout<<"Normal: "<<endl;
        for (Junction* junc: normal) {
            cout<<junc->getSource()->getId()<<"->"<<junc->getTarget()->getId()<<endl;
            adj[junc->getSource()->getId()].push_back(junc->getTarget()->getId());
        }

        lgm->findCircuits(adj);
        //lgm->constructCircuits(adj);

        //Traverse the graph with selected junc
        // JunctionDB *db = new JunctionDB(selectedJunc);
        // db->sortRecordEntry();
    
    } else if (strcmp(argv[1], "split") == 0) {
        const char *lhFn = argv[2];
        const char *chrom = argv[3];
        const char *lhOut = argv[4];
        bool verbose = (strcmp(argv[6], "--verbose") == 0) ? true : false;
        Graph *g = new Graph(lhFn);
        g->calculateHapDepth();
        g->print();

        LocalGenomicMap *lgm = new LocalGenomicMap(g);

        auto segs = lgm->extractReachableGraph(verbose);
        g->writePartialGraph(segs, lhOut);
        g->print();
        cout << "write" << endl;
        g->writeGraph(lhOut);
        cout << "write done" << endl;

    }
}
