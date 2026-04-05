class Set
  include Enumerable

  def initialize(elements = [])
    @hash = {}
    if elements.is_a?(Array)
      elements.each { |elem| @hash[elem] = true }
    end
  end

  def add(element)
    @hash[element] = true
    self
  end

  alias_method :<<, :add

  def delete(element)
    @hash.delete(element)
    self
  end

  def include?(element)
    @hash.include?(element)
  end

  def size
    @hash.size
  end

  alias_method :length, :size

  def empty?
    @hash.empty?
  end

  def clear
    @hash.clear
    self
  end

  def each
    @hash.each do |key, value|
      yield key
    end
    self
  end

  def union(other)
    result = Set.new(to_a)
    other.each { |elem| result.add(elem) }
    result
  end

  alias_method :|, :union

  def difference(other)
    result = Set.new(to_a)
    other.each { |elem| result.delete(elem) }
    result
  end

  alias_method :-, :difference

  def intersection(other)
    result = Set.new
    each { |elem| result.add(elem) if other.include?(elem) }
    result
  end

  alias_method :&, :intersection

  def ==(other)
    return false unless other.is_a?(Set)
    size == other.size && all? { |elem| other.include?(elem) }
  end

  def to_a
    @hash.keys
  end

  def to_s
    "#<Set: {#{to_a.join(', ')}}>"
  end

  def inspect
    to_s
  end
end
