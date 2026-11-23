# typed: strict

class Account
  extend T::Sig

  sig { returns(String) }
  def name = "a"

  sig { params(blk: T.proc.params(account: T.self_type).void).returns(T.self_type) }
  def each_self(&blk)
    yield self
    self
  end
end

class Savings < Account; end

Account.new.then { |a| T.reveal_type(a) } # error: Revealed type: `Account`
Account.new.tap { |a| T.reveal_type(a) } # error: Revealed type: `Account`
Account.new.yield_self { |a| T.reveal_type(a) } # error: Revealed type: `Account`
T.reveal_type(Account.new.then(&:name)) # error: Revealed type: `String`
[1, 2].then { |xs| T.reveal_type(xs) } # error: Revealed type: `[Integer(1), Integer(2)] (2-tuple)`
T.reveal_type(1.tap { |n| T.reveal_type(n) }) # error-with-dupes: Revealed type: `Integer(1)`
Savings.new.each_self { |s| T.reveal_type(s) } # error: Revealed type: `Savings`
