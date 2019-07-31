contract TransferTest {
  function() external payable {
	// This used to cause an ICE
    address(this).transfer;
  }
}
