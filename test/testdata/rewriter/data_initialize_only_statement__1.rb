# typed: true

InlineSig = Data.define(:a) do
  extend T::Sig

  sig { params(a: Integer).void }
  def initialize(a:)
    super
  end
end

RbiSig = Data.define(:a) do
  def initialize(a:)
    super
  end
end

NoSig = Data.define(:a) do
  def initialize(a:)
    super
  end
end

OtherMethod = Data.define(:a) do
  def foo; end
end

InlineSig.new(a: 1)
InlineSig.new(a: "") # error: Expected `Integer` but found `String("")` for argument `a`
InlineSig.new # error: Missing required keyword argument `a` for method `InlineSig#initialize`

RbiSig.new(a: 1)
RbiSig.new(a: "") # error: Expected `Integer` but found `String("")` for argument `a`
RbiSig.new # error: Missing required keyword argument `a` for method `RbiSig#initialize`

NoSig.new(a: 1)
NoSig.new(a: "")
NoSig.new # error: Missing required keyword argument `a` for method `NoSig#initialize`

OtherMethod.new(1)
OtherMethod.new(1, 2) # error: Too many arguments provided for method `OtherMethod#initialize`. Expected: `0..1`, got: `2`
