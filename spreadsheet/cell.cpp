#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell() : impl_(nullptr){}

#define FORMULA_SIGN '='

void Cell::Set(std::string text,SheetInterface& sheet) {
    if(text.size() > 1 && text[0] == FORMULA_SIGN){
        impl_ = std::make_unique<FormulaImpl>(text.substr(1),sheet);
    }
    else {
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }
}

void Cell::Clear() {
    impl_.reset();
    impl_ = nullptr;
}

Cell::Value Cell::GetValue() const {
    if (cashvalue_){
        return cashvalue_.value();
    }
    else{
        if(impl_ == nullptr){
            return 0.0;
        }
        Value rs = impl_->GetValue();
        if (std::holds_alternative<double>(rs))
            cashvalue_ = std::get<double>(rs);
        return rs;
    }
}
std::string Cell::GetText() const {
    return impl_->GetText();
}
std::vector<Position> Cell::GetReferencedCells() const{
    if(impl_==nullptr){
        return std::vector<Position>();
    }
    return impl_->GetReferencedCells();
}
std::vector<Position> Cell::GetUpDependencesCells() const{
    std::vector<Position> rs;
    for (const auto& el : cell_depend_up_){
        rs.push_back(el);
    }
    std::sort(rs.begin(), rs.end());
    return rs;
}
