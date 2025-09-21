#include "threads.h"

#include <algorithm>
#include <chrono>

using namespace std;

// ArithmeticFunction implementation
size_t ArithmeticFunction::requiredArgs() const {
    size_t needed = 0;
    if (!left_operand.has_value()) needed++;
    if (!right_operand.has_value()) needed++;
    return needed;
}

string ArithmeticFunction::description() const {
    string ops[] = {"+", "-", "*", "/"};
    string op_str = ops[static_cast<int>(op)];

    auto wrap = [](const DataValue& val) {
        string str = visit(
            [](const auto& v) {
                if constexpr (is_same_v<decay_t<decltype(v)>, complex<double>>) {
                    ostringstream oss;
                    oss << v.real() << (v.imag() >= 0 ? " + " : " - ") << abs(v.imag()) << "i";
                    return oss.str();
                } else {
                    return to_string(v);
                }
            },
            val);
        return (str.find(' ') != string::npos || str[0] == '-') ? "(" + str + ")" : str;
    };

    if (left_operand.has_value() && right_operand.has_value()) {
        return wrap(left_operand.value()) + " " + op_str + " " + wrap(right_operand.value());
    } else if (left_operand.has_value()) {
        return wrap(left_operand.value()) + " " + op_str + " x";
    } else if (right_operand.has_value()) {
        return "x " + op_str + " " + wrap(right_operand.value());
    } else {
        return "x " + op_str + " y";
    }
}

string ArithmeticFunction::valueToString(const DataValue& val) const {
    return visit(
        [](const auto& v) {
            if constexpr (is_same_v<decay_t<decltype(v)>, complex<double>>) {
                ostringstream oss;
                oss << v.real() << (v.imag() >= 0 ? " + " : " - ") << abs(v.imag()) << "i";
                return oss.str();
            } else {
                return to_string(v);
            }
        },
        val);
}

// BaseThread implementation
BaseThread::BaseThread(int id) : threadId(id), gen(rd()) {}
BaseThread::~BaseThread() {
    stop();
    if (workerThread.joinable()) workerThread.join();
}
void BaseThread::start() { workerThread = thread(&BaseThread::workLoop, this); }
void BaseThread::stop() { shouldStop = true; }
int BaseThread::getId() const { return threadId; }
bool BaseThread::isRunning() const { return !shouldStop && workerThread.joinable(); }
void BaseThread::log(const string& message) {
    cout << "[Thread " << threadId << "] " << message << endl;
}

// DataThread implementation
DataThread::DataThread(int id, int queueCapacity)
    : BaseThread(id),
      dataQueue(make_unique<Queue<DataValue>>(queueCapacity)),
      typeSelector(0, 2),
      intGenerator(DATA_MIN_VALUE, DATA_MAX_VALUE),
      floatGenerator(static_cast<float>(DATA_MIN_VALUE), static_cast<float>(DATA_MAX_VALUE)),
      complexGenerator(static_cast<double>(DATA_MIN_VALUE), static_cast<double>(DATA_MAX_VALUE)) {
    log("Data thread created with queue ID: " + to_string(dataQueue->getId()) +
        ", capacity: " + to_string(queueCapacity));
    start();
}

DataThread::~DataThread() {
    stop();
    if (workerThread.joinable()) workerThread.join();
}
int DataThread::getQueueId() const { return dataQueue->getId(); }
size_t DataThread::getQueueSize() const { return dataQueue->size(); }
bool DataThread::isQueueEmpty() const { return dataQueue->empty(); }
DataValue DataThread::popValue() { return dataQueue->pop(); }
void DataThread::pushValue(const DataValue& value) { dataQueue->push(value); }

void DataThread::workLoop() {
    log("Started working");
    while (!shouldStop) {
        try {
            DataValue value = generateRandomValue();
            dataQueue->push(value);
            logGeneratedValue(value);
            this_thread::sleep_for(chrono::milliseconds(200 + (threadId % 5) * 50));
        } catch (const exception& e) {
            log("Error: " + string(e.what()));
            break;
        }
    }
    log("Finished working");
}

DataValue DataThread::generateRandomValue() {
    int type = typeSelector(gen);
    switch (type) {
        case 0:
            return intGenerator(gen);
        case 1:
            return floatGenerator(gen);
        case 2:
            return complex<double>(complexGenerator(gen), complexGenerator(gen));
        default:
            return 0;
    }
}

void DataThread::logGeneratedValue(const DataValue& value) {
    string message = "Generated: ";
    visit(
        [&message](const auto& v) {
            if constexpr (is_same_v<decay_t<decltype(v)>, complex<double>>) {
                message += to_string(v.real()) + (v.imag() >= 0 ? " + " : " - ") +
                           to_string(abs(v.imag())) + "i";
            } else {
                message += to_string(v);
            }
        },
        value);
    message += " (queue size: " + to_string(dataQueue->size()) + ")";
    log(message);
}

// FunctionThread implementation
FunctionThread::FunctionThread(int id, int queueCapacity)
    : BaseThread(id),
      functionQueue(make_unique<Queue<ArithmeticFunction>>(queueCapacity)),
      operationSelector(0, 3),
      patternSelector(0, 3),
      intConstGenerator(-20, 20),
      floatConstGenerator(-10.0f, 10.0f),
      dataTypeSelector(0, 2) {
    log("Function thread created with queue ID: " + to_string(functionQueue->getId()) +
        ", capacity: " + to_string(queueCapacity));
    start();
}

FunctionThread::~FunctionThread() {
    stop();
    if (workerThread.joinable()) workerThread.join();
}
int FunctionThread::getQueueId() const { return functionQueue->getId(); }
size_t FunctionThread::getQueueSize() const { return functionQueue->size(); }
bool FunctionThread::isQueueEmpty() const { return functionQueue->empty(); }
ArithmeticFunction FunctionThread::popFunction() { return functionQueue->pop(); }

void FunctionThread::workLoop() {
    log("Started working");
    while (!shouldStop) {
        try {
            ArithmeticFunction func = generateRandomFunction();
            functionQueue->push(func);
            logGeneratedFunction(func);
            this_thread::sleep_for(chrono::milliseconds(300 + (threadId % 5) * 75));
        } catch (const exception& e) {
            log("Error: " + string(e.what()));
            break;
        }
    }
    log("Finished working");
}

ArithmeticFunction FunctionThread::generateRandomFunction() {
    ArithmeticFunction func;
    func.op = static_cast<Operation>(operationSelector(gen));
    int pattern = patternSelector(gen);

    switch (pattern) {
        case 1:
            func.right_operand = generateRandomConstant();
            break;
        case 2:
            func.left_operand = generateRandomConstant();
            break;
        case 3:
            func.left_operand = generateRandomConstant();
            func.right_operand = generateRandomConstant();
            break;
    }
    return func;
}

DataValue FunctionThread::generateRandomConstant() {
    int type = dataTypeSelector(gen);
    switch (type) {
        case 0:
            return intConstGenerator(gen);
        case 1:
            return floatConstGenerator(gen);
        case 2:
            return complex<double>(intConstGenerator(gen), intConstGenerator(gen));
        default:
            return 0;
    }
}

void FunctionThread::logGeneratedFunction(const ArithmeticFunction& func) {
    log("Generated function: " + func.description() + " (needs " + to_string(func.requiredArgs()) +
        " args) (queue size: " + to_string(functionQueue->size()) + ")");
}

// ProcessingThread implementation
ProcessingThread::ProcessingThread(int id, atomic<int>& processed, int maxFunctions,
                                   const vector<unique_ptr<DataThread>>& dataThreads,
                                   const vector<unique_ptr<FunctionThread>>& functionThreads)
    : BaseThread(id),
      functionsProcessed(processed),
      maxFunctions(maxFunctions),
      dataThreads(dataThreads),
      functionThreads(functionThreads),
      queueSelector(0, numeric_limits<int>::max()) {
    log("Processing thread created");
    start();
}

void ProcessingThread::workLoop() {
    log("Started processing");
    while (!shouldStop && functionsProcessed.load() < maxFunctions) {
        try {
            if (dataThreads.empty() && functionThreads.empty()) {
                this_thread::sleep_for(chrono::milliseconds(100));
                continue;
            }

            auto [firstIdx, secondIdx] = selectTwoRandomQueues();
            if (firstIdx == -1 || secondIdx == -1) {
                this_thread::sleep_for(chrono::milliseconds(50));
                continue;
            }

            auto dataSize = static_cast<int>(dataThreads.size());
            bool firstIsData = firstIdx < dataSize;
            bool secondIsData = secondIdx < dataSize;

            if (firstIsData && secondIsData) {
                processDataToData(dataThreads[firstIdx].get(), dataThreads[secondIdx].get());
            } else if (!firstIsData && !secondIsData) {
                log("Both queues are function queues, ignoring");
            } else {
                DataThread* dataThread =
                    firstIsData ? dataThreads[firstIdx].get() : dataThreads[secondIdx].get();
                FunctionThread* funcThread = firstIsData
                                                 ? functionThreads[secondIdx - dataSize].get()
                                                 : functionThreads[firstIdx - dataSize].get();
                processFunctionWithData(funcThread, dataThread);
            }

            this_thread::sleep_for(chrono::milliseconds(100 + (threadId % 3) * 50));
        } catch (const exception& e) {
            log("Error: " + string(e.what()));
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
    log("Finished processing");
}

pair<int, int> ProcessingThread::selectTwoRandomQueues() {
    auto totalQueues = static_cast<int>(dataThreads.size() + functionThreads.size());
    if (totalQueues < 2) return {-1, -1};

    uniform_int_distribution<> dist(0, totalQueues - 1);
    int first = dist(gen), second;
    do {
        second = dist(gen);
    } while (second == first);
    return {first, second};
}

void ProcessingThread::processDataToData(DataThread* source, DataThread* dest) {
    if (!source || !dest || source->isQueueEmpty()) return;
    try {
        if (!source->isQueueEmpty()) {
            DataValue value = source->popValue();
            dest->pushValue(value);
            log("Transferred " + valueToString(value) + " from queue " +
                to_string(source->getQueueId()) + " to queue " + to_string(dest->getQueueId()));
        }
    } catch (const exception& e) {
        log("Transfer error: " + string(e.what()));
    }
}

void ProcessingThread::processFunctionWithData(FunctionThread* functionThread,
                                               DataThread* dataThread) {
    if (!functionThread || !dataThread || functionThread->isQueueEmpty()) return;
    try {
        ArithmeticFunction func = functionThread->popFunction();
        size_t argsNeeded = func.requiredArgs();

        if (dataThread->getQueueSize() < argsNeeded) {
            log("Not enough data values for function (need " + to_string(argsNeeded) + ", have " +
                to_string(dataThread->getQueueSize()) + ")");
            return;
        }

        vector<DataValue> args;
        for (size_t i = 0; i < argsNeeded; ++i) args.push_back(dataThread->popValue());

        DataValue result = applyFunction(func, args);
        log(formatFunctionExecution(func, args, result));
        functionsProcessed.fetch_add(1);
    } catch (const exception& e) {
        log("Function application error: " + string(e.what()));
    }
}

DataValue ProcessingThread::applyFunction(const ArithmeticFunction& func,
                                          const vector<DataValue>& args) {
    DataValue left = func.left_operand.has_value() ? func.left_operand.value() : args[0];
    DataValue right = func.right_operand.has_value() ? func.right_operand.value()
                                                     : args[func.left_operand.has_value() ? 0 : 1];

    return visit(
        [&func](const auto& x, const auto& y) -> DataValue {
            using T = common_type_t<decay_t<decltype(x)>, decay_t<decltype(y)>>;
            if constexpr (is_same_v<T, complex<double>>) {
                complex<double> a(x), b(y);
                switch (func.op) {
                    case Operation::ADD:
                        return a + b;
                    case Operation::SUBTRACT:
                        return a - b;
                    case Operation::MULTIPLY:
                        return a * b;
                    case Operation::DIVIDE:
                        if (abs(b) < 1e-10) throw runtime_error("Division by zero");
                        return a / b;
                }
            } else {
                T a = static_cast<T>(x), b = static_cast<T>(y);
                switch (func.op) {
                    case Operation::ADD:
                        return a + b;
                    case Operation::SUBTRACT:
                        return a - b;
                    case Operation::MULTIPLY:
                        return a * b;
                    case Operation::DIVIDE:
                        if (abs(static_cast<double>(b)) < 1e-10)
                            throw runtime_error("Division by zero");
                        return a / b;
                }
            }
            throw runtime_error("Unknown operation");
        },
        left, right);
}

string ProcessingThread::formatFunctionExecution(const ArithmeticFunction& func,
                                                 const vector<DataValue>& args,
                                                 const DataValue& result) {
    string output = "Function: {" + func.description() + "}; parameters: ";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) output += ", ";
        output += valueToString(args[i]);
    }
    return output + "; result: " + valueToString(result);
}

string ProcessingThread::valueToString(const DataValue& val) {
    return visit(
        [](const auto& v) {
            if constexpr (is_same_v<decay_t<decltype(v)>, complex<double>>) {
                ostringstream oss;
                oss << v.real() << (v.imag() >= 0 ? " + " : " - ") << abs(v.imag()) << "i";
                return oss.str();
            } else {
                return to_string(v);
            }
        },
        val);
}