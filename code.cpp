C++#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <optional>

// ==================== Type System ====================

enum class DataType {
    VOID,
    CHAR,
    INT,
    FLOAT,
    STRING,
    // Easily extensible: BOOL, POINTER, ARRAY, etc.
};

std::string toString(DataType type) {
    switch (type) {
        case DataType::VOID:   return "void";
        case DataType::CHAR:   return "char";
        case DataType::INT:    return "int";
        case DataType::FLOAT:  return "float";
        case DataType::STRING: return "string";
        default:               return "unknown";
    }
}

// Widening conversions allowed implicitly (no data loss)
bool canImplicitlyConvert(DataType from, DataType to) {
    if (from == to) return true;

    switch (to) {
        case DataType::INT:
            return from == DataType::CHAR;
        case DataType::FLOAT:
            return from == DataType::CHAR || from == DataType::INT;
        default:
            return false;
    }
}

// ==================== Symbol Table ====================

class SymbolTable {
private:
    std::map<std::string, DataType> symbols;

public:
    bool declare(const std::string& name, DataType type) {
        if (symbols.count(name)) {
            return false; // Already declared
        }
        symbols[name] = type;
        return true;
    }

    std::optional<DataType> lookup(const std::string& name) const {
        auto it = symbols.find(name);
        return (it != symbols.end()) ? std::optional<DataType>{it->second}
                                     : std::nullopt;
    }
};

// ==================== Expression Representation ====================

struct Literal {
    std::variant<char, int, float, std::string> value;

    DataType type() const {
        if (std::holds_alternative<char>(value))     return DataType::CHAR;
        if (std::holds_alternative<int>(value))      return DataType::INT;
        if (std::holds_alternative<float>(value))    return DataType::FLOAT;
        if (std::holds_alternative<std::string>(value)) return DataType::STRING;
        return DataType::VOID; // Should not happen
    }
};

class Expression {
public:
    virtual ~Expression() = default;
    virtual DataType getType(const SymbolTable& symTab) const = 0;
};

class LiteralExpr : public Expression {
private:
    Literal lit;

public:
    explicit LiteralExpr(Literal l) : lit(std::move(l)) {}

    DataType getType(const SymbolTable&) const override {
        return lit.type();
    }
};

class VariableExpr : public Expression {
private:
    std::string name;

public:
    explicit VariableExpr(std::string n) : name(std::move(n)) {}

    DataType getType(const SymbolTable& symTab) const override {
        auto typeOpt = symTab.lookup(name);
        if (!typeOpt.has_value()) {
            throw std::runtime_error("Undeclared variable: " + name);
        }
        return typeOpt.value();
    }

    const std::string& getName() const { return name; }
};

// ==================== Type Checker ====================

class TypeChecker {
private:
    const SymbolTable& symbolTable;
    int currentLine = 0;

    void reportError(const std::string& msg) const {
        std::cerr << "Semantic Error (line " << currentLine << "): " << msg << std::endl;
    }

public:
    explicit TypeChecker(const SymbolTable& st) : symbolTable(st) {}

    void setLine(int line) { currentLine = line; }

    bool checkAssignment(const std::string& varName,
                         const Expression& rhsExpr) {
        // 1. Lookup LHS type
        auto lhsTypeOpt = symbolTable.lookup(varName);
        if (!lhsTypeOpt.has_value()) {
            reportError("Undeclared variable '" + varName + "'");
            return false;
        }
        DataType lhsType = lhsTypeOpt.value();

        // 2. Compute RHS type
        DataType rhsType;
        try {
            rhsType = rhsExpr.getType(symbolTable);
        } catch (const std::runtime_error& e) {
            reportError(e.what());
            return false;
        }

        // 3. Compatibility check
        if (!canImplicitlyConvert(rhsType, lhsType)) {
            reportError("Type mismatch: cannot assign " + toString(rhsType) +
                        " to variable of type " + toString(lhsType));
            return false;
        }

        // Success
        std::cout << "Assignment OK (line " << currentLine << "): "
                  << varName << " (" << toString(lhsType) << ") = "
                  << "expression of type " << toString(rhsType) << std::endl;

        // Optional: In a real compiler, insert implicit cast node here
        return true;
    }
};

// ==================== Demo / Test ====================

int main() {
    SymbolTable symTab;

    // Simulate prior declarations
    symTab.declare("x", DataType::INT);
    symTab.declare("y", DataType::FLOAT);
    symTab.declare("c", DataType::CHAR);
    symTab.declare("msg", DataType::STRING);

    TypeChecker checker(symTab);

    // Test cases
    auto test = [&](int line, const std::string& var, const Expression& expr) {
        checker.setLine(line);
        checker.checkAssignment(var, expr);
        std::cout << std::endl;
    };

    test(10, "x", LiteralExpr(Literal{42}));                    // int = int → OK
    test(20, "y", LiteralExpr(Literal{5}));                     // float = int → OK (coercion)
    test(30, "y", LiteralExpr(Literal{3.14f}));                 // float = float → OK
    test(40, "c", LiteralExpr(Literal{65}));                    // char = int → narrowing → ERROR
    test(50, "x", LiteralExpr(Literal{std::string("hello")}));  // int = string → ERROR
    test(60, "y", VariableExpr("x"));                           // float = int var → OK (coercion)
    test(70, "msg", LiteralExpr(Literal{std::string("hi")}));   // string = string → OK
    test(80, "undefined", LiteralExpr(Literal{10}));            // LHS undeclared → ERROR

    return 0;
}