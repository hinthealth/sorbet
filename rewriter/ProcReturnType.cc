#include "rewriter/ProcReturnType.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/core.h"
#include "rewriter/util/Util.h"

using namespace std;
namespace sorbet::rewriter {

namespace {

const ast::Send *findReturns(const ast::Send *send) {
    while (send != nullptr && send->fun != core::Names::returns()) {
        send = ast::cast_tree<ast::Send>(send->recv);
    }
    return send;
}

// `T.proc...` without `.bind`: a bound proc changes `self` inside the lambda, which a `T.let` can't express.
bool isUnboundProcType(const ast::ExpressionPtr &typeExpr) {
    auto send = ast::cast_tree<ast::Send>(typeExpr);
    while (send != nullptr) {
        if (send->fun == core::Names::bind()) {
            return false;
        }
        if (send->fun == core::Names::proc()) {
            auto recv = ast::cast_tree<ast::UnresolvedConstantLit>(send->recv);
            if (recv != nullptr && recv->cnst == core::Names::Constants::T()) {
                return true;
            }
            auto resolved = ast::cast_tree<ast::ConstantLit>(send->recv);
            return resolved != nullptr && resolved->symbol() == core::Symbols::T();
        }
        send = ast::cast_tree<ast::Send>(send->recv);
    }
    return false;
}

bool isArrowLambda(const ast::ExpressionPtr &expr) {
    auto send = ast::cast_tree<ast::Send>(expr);
    if (send == nullptr || send->fun != core::Names::lambda() || send->hasNonBlockArgs() || send->block() == nullptr) {
        return false;
    }
    auto recv = ast::cast_tree<ast::ConstantLit>(send->recv);
    return recv != nullptr && recv->symbol() == core::Symbols::Kernel();
}

ast::ExpressionPtr *lastExpression(ast::ExpressionPtr &body) {
    if (auto insSeq = ast::cast_tree<ast::InsSeq>(body)) {
        return &insSeq->expr;
    }
    return &body;
}

} // namespace

void ProcReturnType::run(core::MutableContext ctx, ast::MethodDef *methodDef, const ast::ExpressionPtr *prevStat) {
    if (prevStat == nullptr) {
        return;
    }
    auto *sig = ASTUtil::castSig(*prevStat);
    if (sig == nullptr || sig->block() == nullptr) {
        return;
    }
    auto *returns = findReturns(ast::cast_tree<ast::Send>(sig->block()->body));
    if (returns == nullptr || returns->numPosArgs() != 1 || !isUnboundProcType(returns->getPosArg(0))) {
        return;
    }

    auto *last = lastExpression(methodDef->rhs);
    if (!isArrowLambda(*last)) {
        return;
    }

    auto loc = last->loc();
    *last = ast::MK::Let(loc, move(*last), returns->getPosArg(0).deepCopy());
}

} // namespace sorbet::rewriter
