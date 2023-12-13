#pragma once

#include "common.h"
#include "formula.h"
#include <memory>
#include <optional>

template <>
struct std::hash<Position>
{
    std::size_t operator()(const Position& position) const noexcept
    {
        return (position.row < 12) + position.col;
    }
};

using Value = CellInterface::Value;

class Impl {
public:
    virtual ~Impl() = default;
    virtual void Set(std::string text) = 0;
    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual void Clear() = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

class EmptyImpl : public Impl {
public:
    EmptyImpl() = default;
    virtual ~EmptyImpl() = default;

    virtual void Set(std::string text) override
    {}

    virtual void Clear() override
    {}

    virtual Value GetValue() const override{
        return 0.0;
    }

    virtual std::string GetText() const override{
        return {};
    }

    virtual std::vector<Position> GetReferencedCells() const override    {
        return {};
    }
};
class TextImpl : public Impl{
public:
    TextImpl() = default;
    ~TextImpl() = default;

    explicit TextImpl(std::string value) :
        value_(std::move(value)){
    }
    void Set(std::string text) override{
        value_ = std::move(text);
    }
    Value GetValue()const override{
        if(value_[0] == '\''){
            return value_.substr(1);
        }
        return value_;
    }
    std::string GetText() const override{
        return value_;
    }
    void Clear() override{
        value_.clear();
    }
    std::vector<Position> GetReferencedCells() const override{
        return {};
    }

private:
    std::string value_;
};

class FormulaImpl : public Impl{
public:
    FormulaImpl(const SheetInterface& sheet) :
        sheet_(sheet)
        {};

    ~FormulaImpl() = default;

    explicit FormulaImpl(std::string value, const SheetInterface& sheet) :
        sheet_(sheet){
        formula_ = ParseFormula(value);
    }

    void Set(std::string text) override{
        formula_ = ParseFormula(text);
    }

    Value GetValue()const override{
        auto result = formula_->Evaluate(sheet_);
        if(std::holds_alternative<double>(result)){
            return std::get<double>(result);
        }
        return std::get<FormulaError>(result);
    }

    std::string GetText() const override{
        return FORMULA_SIGN + formula_->GetExpression();
    }

    void Clear() override{
        formula_.reset();
    }

    std::vector<Position> GetReferencedCells() const override {
        return formula_->GetReferencedCells();
    }

private:
    std::unique_ptr<FormulaInterface> formula_;
    const SheetInterface& sheet_;
};

class Cell : public CellInterface {
public:
    friend class Sheet;
    Cell();
    ~Cell() = default;
    void Set(std::string text, SheetInterface& sheet);
    void Clear();
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    std::vector<Position> GetUpDependencesCells() const;

private:
    std::unique_ptr<Impl> impl_;
    mutable std::optional<double> cashvalue_;
    std::unordered_set<Position> cell_depend_up_;
};
