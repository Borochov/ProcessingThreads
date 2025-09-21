#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "threads.h"

using namespace std;

void printUsage(const char* programName) {
    cout << "Usage: " << programName << " <NF> <ND> <NP> <NA>" << endl;
    cout << "  NF - number of function threads" << endl;
    cout << "  ND - number of data threads" << endl;
    cout << "  NP - number of processing threads" << endl;
    cout << "  NA - number of applied functions (stop condition)" << endl;
    cout << endl;
    cout << "Example: " << programName << " 2 3 2 10" << endl;
}

int calculateQueueCapacity(int producers) {
    return producers * 10;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printUsage(argv[0]);
        return 1;
    }

    try {
        int NF = stoi(argv[1]);  // Number of function threads
        int ND = stoi(argv[2]);  // Number of data threads
        int NP = stoi(argv[3]);  // Number of processing threads
        int NA = stoi(argv[4]);  // Number of applied functions

        if (NF < 0 || ND < 0 || NP < 0 || NA < 0) {
            cerr << "Error: All parameters must be non-negative." << endl;
            return 1;
        }

        cout << "Starting Processing Threads Demo" << endl;
        cout << "=================================" << endl;
        cout << "Function threads: " << NF << endl;
        cout << "Data threads: " << ND << endl;
        cout << "Processing threads: " << NP << endl;
        cout << "Functions to apply: " << NA << endl;
        cout << endl;

        // Global counter for applied functions
        atomic<int> functionsProcessed{0};

        // Thread pools
        vector<unique_ptr<DataThread>> dataThreads;
        vector<unique_ptr<FunctionThread>> functionThreads;
        vector<unique_ptr<ProcessingThread>> processingThreads;

        // Calculate queue capacities to avoid deadlocks
        int dataQueueCapacity = calculateQueueCapacity(ND);
        int functionQueueCapacity = calculateQueueCapacity(NF);

        cout << "Calculated queue capacities:" << endl;
        cout << "  Data queues: " << dataQueueCapacity << endl;
        cout << "  Function queues: " << functionQueueCapacity << endl;
        cout << endl;

        // Create data threads
        cout << "Creating " << ND << " data threads..." << endl;
        for (int i = 0; i < ND; ++i) {
            dataThreads.push_back(make_unique<DataThread>(i + 1, dataQueueCapacity));
        }

        // Create function threads
        cout << "Creating " << NF << " function threads..." << endl;
        for (int i = 0; i < NF; ++i) {
            functionThreads.push_back(make_unique<FunctionThread>(i + 100, functionQueueCapacity));
        }

        // Allow some time for data and function generation
        cout << "Allowing threads to generate initial data..." << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));

        // Create processing threads
        cout << "Creating " << NP << " processing threads..." << endl;
        for (int i = 0; i < NP; ++i) {
            processingThreads.push_back(make_unique<ProcessingThread>(
                i + 200, functionsProcessed, NA, dataThreads, functionThreads));
        }

        cout << "All threads started. Processing..." << endl;
        cout << endl;

        // Monitor progress
        auto startTime = chrono::steady_clock::now();
        while (functionsProcessed.load() < NA) {
            this_thread::sleep_for(chrono::milliseconds(500));

            auto currentTime = chrono::steady_clock::now();
            auto elapsed = chrono::duration_cast<chrono::seconds>(currentTime - startTime);

            cout << "Progress: " << functionsProcessed.load() << "/" << NA
                 << " functions processed (elapsed: " << elapsed.count() << "s)" << endl;

            // Safety timeout (optional)
            if (elapsed.count() > 60) {  // 60 seconds timeout
                cout << "Timeout reached. Stopping..." << endl;
                break;
            }
        }

        cout << endl;
        cout << "Stopping all threads..." << endl;

        // Stop all threads
        for (auto& thread : processingThreads) {
            thread->stop();
        }
        for (auto& thread : dataThreads) {
            thread->stop();
        }
        for (auto& thread : functionThreads) {
            thread->stop();
        }

        // Wait for all threads to finish
        cout << "Waiting for threads to finish..." << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));

        cout << endl;
        cout << "Final Statistics:" << endl;
        cout << "=================" << endl;
        cout << "Functions processed: " << functionsProcessed.load() << endl;

        // Display final queue sizes
        cout << "\nFinal queue sizes:" << endl;
        for (size_t i = 0; i < dataThreads.size(); ++i) {
            cout << "Data thread " << (i + 1) << " (queue " << dataThreads[i]->getQueueId()
                 << "): " << dataThreads[i]->getQueueSize() << " values" << endl;
        }
        for (size_t i = 0; i < functionThreads.size(); ++i) {
            cout << "Function thread " << (i + 100) << " (queue "
                 << functionThreads[i]->getQueueId() << "): " << functionThreads[i]->getQueueSize()
                 << " functions" << endl;
        }

        auto endTime = chrono::steady_clock::now();
        auto totalElapsed = chrono::duration_cast<chrono::seconds>(endTime - startTime);
        cout << "\nTotal execution time: " << totalElapsed.count() << " seconds" << endl;

    } catch (const invalid_argument& e) {
        cerr << "Error: Invalid argument - " << e.what() << endl;
        printUsage(argv[0]);
        return 1;
    } catch (const out_of_range& e) {
        cerr << "Error: Number out of range - " << e.what() << endl;
        return 1;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    cout << "\nFinished!" << endl;
    return 0;
}
