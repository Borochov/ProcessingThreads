#ifndef THREADS_H
#define THREADS_H

#include <atomic>
#include <complex>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <sstream>
#include <thread>
#include <variant>
#include <vector>

#include "queue.h"

// Data types that threads can generate
using DataValue = std::variant<int, float, std::complex<double>>;

// Data generation range
constexpr int DATA_MIN_VALUE = -100;
constexpr int DATA_MAX_VALUE = 100;

// Arithmetic operations
enum class Operation { ADD, SUBTRACT, MULTIPLY, DIVIDE };

// Function representation
struct ArithmeticFunction {
    Operation op;
    std::optional<DataValue> left_operand;   // if present, use this as left operand
    std::optional<DataValue> right_operand;  // if present, use this as right operand

    // How many arguments this function needs from the data queue
    size_t requiredArgs() const;

    // String representation of the function
    std::string description() const;

   private:
    std::string valueToString(const DataValue& val) const;
};

// Forward declarations
class DataThread;
class FunctionThread;

// Base thread class
class BaseThread {
   protected:
    int threadId;
    std::thread workerThread;
    std::atomic<bool> shouldStop{false};

    // Random number generation
    std::random_device rd;
    std::mt19937 gen;

   public:
    BaseThread(int id);
    virtual ~BaseThread();

    void start();
    void stop();
    int getId() const;
    bool isRunning() const;

   protected:
    virtual void workLoop() = 0;
    void log(const std::string& message);
};

// Data generation thread
class DataThread : public BaseThread {
   public:
    DataThread(int id, int queueCapacity = 50);
    ~DataThread();

    int getQueueId() const;
    size_t getQueueSize() const;
    bool isQueueEmpty() const;

    // For testing - consume a value from the queue
    DataValue popValue();
    void pushValue(const DataValue& value);

   protected:
    void workLoop() override;

   private:
    std::unique_ptr<Queue<DataValue>> dataQueue;
    // Random generators for different data types
    std::uniform_int_distribution<> typeSelector;
    std::uniform_int_distribution<> intGenerator;
    std::uniform_real_distribution<float> floatGenerator;
    std::uniform_real_distribution<double> complexGenerator;

    DataValue generateRandomValue();
    void logGeneratedValue(const DataValue& value);
};

// Function generation thread
class FunctionThread : public BaseThread {
   public:
    FunctionThread(int id, int queueCapacity = 50);
    ~FunctionThread();

    int getQueueId() const;
    size_t getQueueSize() const;
    bool isQueueEmpty() const;

    // For testing - consume a function from the queue
    ArithmeticFunction popFunction();

   protected:
    void workLoop() override;

   private:
    std::unique_ptr<Queue<ArithmeticFunction>> functionQueue;
    // Random generators for function creation
    std::uniform_int_distribution<> operationSelector;  // 0-3 for +,-,*,/
    std::uniform_int_distribution<> patternSelector;    // 0-3 for different function patterns
    std::uniform_int_distribution<> intConstGenerator;  // for constant values
    std::uniform_real_distribution<float> floatConstGenerator;
    std::uniform_int_distribution<> dataTypeSelector;  // for choosing constant types

    ArithmeticFunction generateRandomFunction();
    DataValue generateRandomConstant();
    void logGeneratedFunction(const ArithmeticFunction& func);
};

// Processing thread - performs operations between queues
class ProcessingThread : public BaseThread {
   public:
    ProcessingThread(int id, std::atomic<int>& processed, int maxFunctions,
                     const std::vector<std::unique_ptr<DataThread>>& dataThreads,
                     const std::vector<std::unique_ptr<FunctionThread>>& functionThreads);

   protected:
    void workLoop() override;

   private:
    std::atomic<int>& functionsProcessed;
    int maxFunctions;
    std::uniform_int_distribution<> queueSelector;
    // References to the actual thread pools
    const std::vector<std::unique_ptr<DataThread>>& dataThreads;
    const std::vector<std::unique_ptr<FunctionThread>>& functionThreads;

    std::pair<int, int> selectTwoRandomQueues();
    void processDataToData(DataThread* source, DataThread* dest);
    void processFunctionWithData(FunctionThread* functionThread, DataThread* dataThread);
    DataValue applyFunction(const ArithmeticFunction& func, const std::vector<DataValue>& args);
    DataValue addValues(const DataValue& a, const DataValue& b);
    DataValue subtractValues(const DataValue& a, const DataValue& b);
    DataValue multiplyValues(const DataValue& a, const DataValue& b);
    DataValue divideValues(const DataValue& a, const DataValue& b);
    std::string formatFunctionExecution(const ArithmeticFunction& func,
                                        const std::vector<DataValue>& args,
                                        const DataValue& result);
    std::string valueToString(const DataValue& val);
};

#endif  // THREADS_H
