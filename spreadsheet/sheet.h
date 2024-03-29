#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>
#include <memory>

class Sheet : public SheetInterface {
public:

    void SetCell(Position pos, std::string text) override;
    void InsertEmptyCell(Position pos);
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;
    void ClearCell(Position pos) override;
    Size GetPrintableSize() const override;
    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:

    std::unordered_map<Position,std::unique_ptr<CellInterface>>  table_;
    Size table_size_;

    Size SetTableSize(const Position&);
    void ClearDependences(const Position&) ;
    void InsertEmpty(const Position&);
    void RestoreDependences(const Position&);
    void ResetCache(const Position&);
    bool CheckCircularDependences(const Position&);

    void IsValidPos(const Position& , const std::string&) const;
    int GetRows(const Position&);
    int GetCol(const Position& ,int);
    bool CheckCircular(const Position& check_pos, const Position& start_pos,
                  std::unordered_set<Position>& cell);

};
