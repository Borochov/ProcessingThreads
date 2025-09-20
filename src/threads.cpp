#include "threads.h"

#include <chrono>

// ArithmeticFunction implementation
size_t ArithmeticFunction::requiredArgs() const {
    size_t needed = 0;
    if (!left_operand.has_value()) needed++;
    if (!right_operand.has_value()) needed++;
    return needed;
}

std::string ArithmeticFunction::description() const {
    std::string op_str;
    switch (op) {
        case Operation::ADD:
            op_str = "+";
            break;
        case Operation::SUBTRACT:
            op_str = "-";
            break;
        case Operation::MULTIPLY:
            op_str = "*";
            break;
        case Operation::DIVIDE:
            op_str = "/";
            break;
    }

    if (left_operand.has_value() && right_operand.has_value()) {
        return valueToString(left_operand.value()) + " " + op_str + " " +
               valueToString(right_operand.value());
    } else if (left_operand.has_value()) {
        return valueToString(left_operand.value()) + " " + op_str + " x";
    } else if (right_operand.has_value()) {
        return "x " + op_str + " " + valueToString(right_operand.value());
    } else {
        return "x " + op_str + " y";
    }
}

std::string ArithmeticFunction::valueToString(const DataValue& val) const {
    return std::visit(
        [](const auto& v) -> std::string {
            if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::complex<double>>) {
                std::ostringstream oss;
                double real_part = v.real();
                double imag_part = v.imag();

                oss << real_part;
                if (imag_part >= 0) {
                    oss << " + " << imag_part;
                } else {
                    oss << " - " << (-imag_part);
                }
                oss << " * i";
                return oss.str();
            } else {
                return std::to_string(v);
            }
        },
        val);
}

// BaseThread implementation
BaseThread::BaseThread(int id) : threadId(id), gen(rd()) {}

BaseThread::~BaseThread() {
    stop();
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void BaseThread::start() { workerThread = std::thread(&BaseThread::workLoop, this); }

void BaseThread::stop() { shouldStop = true; }

int BaseThread::getId() const { return threadId; }

bool BaseThread::isRunning() const { return !shouldStop && workerThread.joinable(); }

void BaseThread::log(const std::string& message) {
    std::cout << "[Thread " << threadId << "] " << message << std::endl;
}

// DataThread implementation
DataThread::DataThread(int id)
    : BaseThread(id),
      dataQueue(std::make_unique<Queue<DataValue>>()),
      typeSelector(0, 2),
      intGenerator(-1000, 1000),
      floatGenerator(-100.0f, 100.0f),
      complexGenerator(-50.0, 50.0) {
    log("Data thread created with queue ID: " + std::to_string(dataQueue->getId()));
    start();
}

int DataThread::getQueueId() const { return dataQueue->getId(); }

size_t DataThread::getQueueSize() const { return dataQueue->size(); }

bool DataThread::isQueueEmpty() const { return dataQueue->empty(); }

DataValue DataThread::popValue() { return dataQueue->pop(); }

void DataThread::workLoop() {
    log("Started working");

    while (!shouldStop) {
        try {
            DataValue value = generateRandomValue();
            dataQueue->push(value);

            logGeneratedValue(value);

            // Variable sleep time based on thread ID to create different patterns
            std::this_thread::sleep_for(std::chrono::milliseconds(200 + (threadId % 5) * 50));

        } catch (const std::exception& e) {
            log("Error: " + std::string(e.what()));
            break;
        }
    }

    log("Finished working");
}

DataValue DataThread::generateRandomValue() {
    int data_type = typeSelector(gen);

    switch (data_type) {
        case 0:  // int
            return intGenerator(gen);
        case 1:  // float
            return floatGenerator(gen);
        case 2:  // complex
            return std::complex<double>(complexGenerator(gen), complexGenerator(gen));
        default:
            return 0;  // fallback
    }
}

void DataThread::logGeneratedValue(const DataValue& value) {
    std::string message = "Generated: ";

    // Use std::visit with proper handling for each type
    std::visit(
        [&message](const auto& v) {
            if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::complex<double>>) {
                // Handle complex numbers specially
                message += "(" + std::to_string(v.real()) + "," + std::to_string(v.imag()) + ")";
            } else {
                // Handle int and float
                message += std::to_string(v);
            }
        },
        value);

    message += " (queue size: " + std::to_string(dataQueue->size()) + ")";
    log(message);
}

// FunctionThread implementation
FunctionThread::FunctionThread(int id)
    : BaseThread(id),
      functionQueue(std::make_unique<Queue<ArithmeticFunction>>()),
      operationSelector(0, 3),
      patternSelector(0, 3),
      intConstGenerator(-20, 20),
      floatConstGenerator(-10.0f, 10.0f),
      dataTypeSelector(0, 2) {
    log("Function thread created with queue ID: " + std::to_string(functionQueue->getId()));
    start();
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

            // Variable sleep time, slightly slower than data threads
            std::this_thread::sleep_for(std::chrono::milliseconds(300 + (threadId % 5) * 75));

        } catch (const std::exception& e) {
            log("Error: " + std::string(e.what()));
            break;
        }
    }

    log("Finished working");
}

ArithmeticFunction FunctionThread::generateRandomFunction() {
    ArithmeticFunction func;

    // Choose operation
    func.op = static_cast<Operation>(operationSelector(gen));

    // Choose pattern:
    // 0: x op y (needs 2 args from data queue)
    // 1: x op constant (needs 1 arg from data queue)
    // 2: constant op x (needs 1 arg from data queue)
    // 3: constant op constant (needs 0 args from data queue)
    int pattern = patternSelector(gen);

    switch (pattern) {
        case 0:  // x op y
            // Both operands come from data queue
            break;

        case 1:  // x op constant
            func.right_operand = generateRandomConstant();
            break;

        case 2:  // constant op x
            func.left_operand = generateRandomConstant();
            break;

        case 3:  // constant op constant
            func.left_operand = generateRandomConstant();
            func.right_operand = generateRandomConstant();
            break;
    }

    return func;
}

DataValue FunctionThread::generateRandomConstant() {
    int type = dataTypeSelector(gen);

    switch (type) {
        case 0:  // int constant
            return intConstGenerator(gen);
        case 1:  // float constant
            return floatConstGenerator(gen);
        case 2:  // complex constant
            return std::complex<double>(static_cast<double>(intConstGenerator(gen)),
                                        static_cast<double>(intConstGenerator(gen)));
        default:
            return 0;
    }
}

void FunctionThread::logGeneratedFunction(const ArithmeticFunction& func) {
    std::string message = "Generated function: " + func.description();
    message += " (needs " + std::to_string(func.requiredArgs()) + " args)";
    message += " (queue size: " + std::to_string(functionQueue->size()) + ")";
    log(message);
}