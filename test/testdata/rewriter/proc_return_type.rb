# typed: strict

class ProcReturnType
  extend T::Sig

  sig { returns(T.proc.params(x: Integer).returns(String)) }
  def labeled
    ->(x) do
      T.reveal_type(x) # error: Revealed type: `Integer`
      x.to_s
    end
  end

  sig { returns(T.proc.params(x: Integer).returns(String)) }
  def after_statements
    prefix = "n"
    ->(x) do
      T.reveal_type(x) # error: Revealed type: `Integer`
      "#{prefix}#{x}"
    end
  end

  sig { returns(T.proc.params(x: Integer).returns(String)) }
  def wrong_return
    ->(x) { x } # error: Expected `String` but found `Integer` for block result type
  end


  sig { returns(Integer) }
  def not_a_proc
    1
  end
end
