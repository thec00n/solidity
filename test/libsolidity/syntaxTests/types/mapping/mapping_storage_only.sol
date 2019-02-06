contract Test {
    struct S1 {
        mapping(uint => S2) x;
    }
    struct S2 {
        mapping(uint => S3) y;
    }
    struct S3 {
        address addr;
    }
    function addQuestion(S2 memory _x) public returns (bool result) {
    }
}
// ----
// TypeError: (191-203): Data location "memory" is not allowed for mappings.
