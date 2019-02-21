pragma experimental SMTChecker;

contract C
{
	function f(uint[] memory a, uint[] memory b) internal pure {
		require(a[0] == 2);
		b[0] = 1;
		// Fails because b == a is possible.
		assert(a[0] == 2);
		assert(b[0] == 1);
	}
}
// ----
// Warning: (183-200): Assertion violation happens here
