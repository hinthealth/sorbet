#ifndef SORBET_REWRITER_PROC_RETURN_TYPE_H
#define SORBET_REWRITER_PROC_RETURN_TYPE_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * When a method's sig returns a `T.proc` type and the method ends in a `->` lambda, this wraps the lambda in a
 * `T.let` with that type:
 *
 *   sig { returns(T.proc.params(x: Integer).returns(String)) }
 *   def to_label
 *     ->(x) { x.to_s }
 *   end
 *
 * becomes
 *
 *   def to_label
 *     T.let(->(x) { x.to_s }, T.proc.params(x: Integer).returns(String))
 *   end
 *
 * The resolver already turns `T.let(->(...) {...}, T.proc...)` into a lambda whose parameters take the proc's types, so
 * this gives the lambda's parameters the types the sig already states instead of `T.untyped`.
 */
class ProcReturnType final {
public:
    static void run(core::MutableContext ctx, ast::MethodDef *methodDef, const ast::ExpressionPtr *prevStat);

    ProcReturnType() = delete;
};

} // namespace sorbet::rewriter

#endif
