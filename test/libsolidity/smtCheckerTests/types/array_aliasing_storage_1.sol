pragma experimental SMTChecker;

contract C
{
	function f(uint[] storage a, uint[] storage b) internal {
		require(a[0] == 2);
		b[0] = 1;
		// Fails because b == a is possible.
		assert(a[0] == 2);
		assert(b[0] == 1);
	}
}
// ----
// Warning: (180-197): Assertion violation happens here
