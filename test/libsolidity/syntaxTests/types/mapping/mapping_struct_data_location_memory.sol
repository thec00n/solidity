pragma experimental ABIEncoderV2;
contract C {
    struct S { mapping(uint => uint) a; }
    function f(S memory) public {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (104-112): Recursive type is not allowed to live ouside storage.
