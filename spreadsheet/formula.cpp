#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe){
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression) try : ast_(ParseFormulaAST(expression))
    {}
    catch (const std::exception& re){
        throw FormulaException(re.what());
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        try{
            auto a = [&sheet](const Position& pos)->CellInterface*{
                return const_cast<CellInterface*>(sheet.GetCell(pos));
            };
            auto rs = ast_.Execute(a);
            return rs;
        }
        catch(FormulaError& err){
            return err;
        }
    }

    std::string GetExpression() const override {
        try{
            std::ostringstream str;
            ast_.PrintFormula(str);
            return str.str();
        }
        catch(FormulaException()){
            throw FormulaException("");
        }
    }

    std::vector<Position> GetReferencedCells() const override {
        const std::forward_list<Position> formulacells(ast_.GetCells());
        if (formulacells.empty()){
            return {};
        }
        else{
            std::vector<Position> rs{ formulacells.begin(), formulacells.end() };
            return { rs.begin(), std::unique(rs.begin(), rs.end()) };
        }
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
