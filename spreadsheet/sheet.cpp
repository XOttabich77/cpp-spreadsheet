#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value)
{
    std::visit(
        [&](const auto& x) {
            output << x;
        },
        value);
    return output;
}

void Sheet::IsValidPos(const Position& pos, const std::string& str) const{
    if(!pos.IsValid()){
        throw InvalidPositionException(str);
    }
}

void Sheet::SetCell(Position pos, std::string text) {    
    IsValidPos(pos,"SetCell Invalid position:: Set Cell"s);
    std::unique_ptr<CellInterface> tempcellptr = std::make_unique<Cell>();
    Cell* tempcell = static_cast<Cell*>(tempcellptr.get());
    tempcell->Set(std::move(text),*this);
    std::unique_ptr<CellInterface> old_cellptr;
    if (table_.count(pos)){
        ClearDependences(pos);
        old_cellptr = std::move(table_[pos]);
        Cell* old_cell_ptr = static_cast<Cell*>(old_cellptr.get());
        tempcell->cell_depend_up_ = old_cell_ptr->cell_depend_up_;
    }
    table_[pos] = std::move(tempcellptr);
    if (!CheckCircularDependences(pos)){
        if (old_cellptr){
            table_[pos] = std::move(old_cellptr);
            RestoreDependences(pos);
        }
        else{
            table_.erase(pos);
        }
        throw CircularDependencyException("#CIRC!");
    }
    RestoreDependences(pos);
    ResetCache(pos);
    table_size_.cols <= pos.col ? table_size_.cols = pos.col + 1 : 0;
    table_size_.rows <= pos.row ? table_size_.rows = pos.row + 1 : 0;
}

const CellInterface* Sheet::GetCell(Position pos) const {
    IsValidPos(pos,"SetCell Invalid position:: Get Cell const"s);
    if(table_.find(pos)!=table_.end()){
        return table_.at(pos).get();
    }
    return nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    IsValidPos(pos,"SetCell Invalid position:: Get Cell"s);
    if(table_.count(pos)){
        return table_.at(pos).get();
    }
    return nullptr;
}

int Sheet::GetRows(const Position& old_pos){
    if(old_pos.row + 1 != table_size_.rows){
        for(int row = table_size_.rows - 1; row >= 0; --row){
            for(int col = table_size_.cols - 1; col >= 0; --col){
                if(table_.count({row, col})){
                    return row + 1;
                }
            }
     }
    }
    return table_size_.rows;
}

int Sheet::GetCol(const Position& old_pos,int rows){
    if(old_pos.col + 1 == table_size_.cols){
        for(int col = table_size_.cols - 1; col >= 0; --col){
            for(int row = rows - 1 ; row >= 0; --row){
                if(table_.count({row, col})){
                    return col + 1;
                }
            }
        }
    }
    return table_size_.cols;
}

Size Sheet::SetTableSize(const Position& old_pos){
    Size new_size;
    new_size.rows = GetRows(old_pos);
    new_size.cols = GetCol(old_pos, new_size.rows);
    return new_size;
}

void Sheet::ClearCell(Position pos) {
    IsValidPos(pos,"SetCell Invalid position:: Clear Cell const"s);

    if(table_.count(pos)){
        ClearDependences(pos);
        ResetCache(pos);
        table_.erase(pos);
        table_size_ = SetTableSize(pos);
    }
}
Size Sheet::GetPrintableSize() const {
    return table_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for(int r = 0; r < table_size_.rows; ++r){
        for( int c = 0; c < table_size_.cols; ++c){
            Position pos{r,c};
            if(table_.count(pos)){
                output << table_.at(pos)->GetValue();
            }
            if(c < table_size_.cols - 1){
                output <<'\t';
            }
        }
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for(int r = 0; r < table_size_.rows; ++r){
        for( int c = 0; c < table_size_.cols; ++c){
            Position pos{r,c};
            if(table_.count(pos)){
                output << table_.at(pos)->GetText();
            }
            if(c < table_size_.cols - 1){
                output <<'\t';
            }
        }
        output << '\n';
    }
}

void Sheet::ClearDependences(const Position& pos){
    if (!table_.count(pos)){
        return;
    }
    Cell& del_cell = static_cast<Cell&>(*table_.at(pos));
    for (const auto& current_pos : del_cell.GetReferencedCells()){
        Cell& current_cell = static_cast<Cell&>(*table_.at(current_pos));
        current_cell.cell_depend_up_.erase(pos);
    }
}

void Sheet::InsertEmpty(const Position& pos){
    IsValidPos(pos,"SetCell Invalid position:: InsertEmpty"s);
    table_[pos] = std::make_unique<Cell>();
    if ((pos.row + 1) > table_size_.rows){
        table_size_.rows = pos.row + 1;
    }
    if ((pos.col + 1) > table_size_.cols){
        table_size_.cols = pos.col + 1;
    }
}

void Sheet::RestoreDependences(const Position& pos){
    if (!table_.count(pos)){
        return;
    }
    for (const auto& current_pos : table_.at(pos)->GetReferencedCells()){
        if (!table_.count(current_pos)){
            InsertEmpty(current_pos);
        }
        Cell& current_cell = static_cast<Cell&>(*table_.at(current_pos));
        current_cell.cell_depend_up_.insert(pos);
    }
}

void Sheet::ResetCache(const Position& pos){
    if (!table_.count(pos)){
        return;
    }
    Cell& reset_cell = static_cast<Cell&>(*table_.at(pos));
    reset_cell.cashvalue_.reset();
    for (const auto& current_pos : reset_cell.cell_depend_up_){
        if (table_.count(current_pos)){
            Cell& current_cell = static_cast<Cell&>(*table_.at(current_pos));
            if (current_cell.cashvalue_){
                ResetCache(current_pos);
            }
        }
    }
}

bool Sheet::CheckCircularDependences(const Position& pos){
    std::unordered_set<Position> cell;
    return CheckCircular(pos, pos, cell);
}

bool Sheet::CheckCircular(const Position& pos,const Position& start_pos,std::unordered_set<Position>& cell){
    if (!table_.count(pos)){
        return true;
    }
    Cell& check_cell = static_cast<Cell&>(*table_.at(pos));
    cell.insert(pos);
    for (const auto& current_pos : check_cell.GetReferencedCells()){
        if (current_pos == start_pos){
            return false;
        }
        if (!cell.count(current_pos)){
            if (!CheckCircular(current_pos, start_pos, cell)){
                return false;
            }
        }
    }
    return true;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
