# Testing

CrowNet is continuously tested by unit tests and fingerprint tests. All tests can be found in the ![test directory](../../crownet/crownet/tests).


TODO add more information here

- ![unit tests](gtest.md)
- ![fingerprint tests](opp_test.md)



## Tested configurations

### SimuLTE based on OMNeT++ 5.1 (as reference)

| Component     | Version       | Tag         | Comments |
| ------------- |---------------| ----------- | -------- |
| OMNeT++       | 5.1.1         | omnet-5.1.1 | Image: sam-dev.cs.hm.edu:5023/rover/rover-main/omnetpp:omnetpp-5.1.1 |
| inet          | 3.6.4         |   v3.6.4    | |
| SimuLTE       |  1.0.0      |   hm_omnet5_1_1_inet_3_6_4 | Commit: 9b1d71360f5c8034e038e30620500289722c0c52 |
