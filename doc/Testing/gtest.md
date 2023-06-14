# GoogleTest and GoogleMock

The GoogleTest library is used to test C++ code and the GoogleMock Library is used to mock dependencies inside those tests.
We are currently using the version 1.10, but facing an issue, that the new syntax for mocking in GoogleMock does not work.

For more documentation about GoogleTest and Google mock please read the [official documentation](https://github.com/google/googletest/tree/master/docs)

Unfortunately the latest version is having a new syntax. But there is a [chromium page](https://chromium.googlesource.com/external/github.com/google/googletest/+/refs/tags/release-1.8.0/googlemock/docs/ForDummies.md) for the old syntax

To run the GoogleTests in our project navigate to `crownet/crownet` run them with the omnetpp container via `omnetpp exec make make-test MODE=debug` 

## testing

To write a new GoogleTest class create a file named `<unit_name>Test.cc` in the test directory.
For our project we also inherit from BaseOppTest to get some functionalities for the omnet library.

### example

```c++
#include "main_test.h" // includes #include "gtest/gtest.h"
#include "MyUnit.h"

//get some omnetpp utils
class MyUnitTest : public BaseOppTest { 
 public:
    MyUnitTest() {}
 protected:
    MyUnit foo; // some default state to use in alll testcaste
};

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
  MyUnit unit = new MyUnit()
  // Expect two strings not to be equal.
  EXPECT_STRNE(myUnit.returnString("Hello"), "World");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
  int ret = foo.executTestFunction();
  EXPECT_EQ(ret, 42);
}
```

### Assertions

There are two types of assertions on that is fatal (<b>ASSERT</b>) and one that is not (<b>EXPECT</b>). The fatal assertion stops the current test if the expected condition is false. The non-fatal one continues the test and marks it as failed.

### Basic Assertions
For basic true/false conditions.
| Fatal assertion  | Nonfatal assertion | Verifies |
|---|---|---|
| ASSERT_TRUE(condition); | EXPECT_TRUE(condition); | condition is true |
| ASSERT_FALSE(condition); | EXPECT_FALSE(condition); | condition is false |

### Binary Comparison
For comparing two values.
| Fatal assertion  | Nonfatal assertion | Verifies |
|---|---|---|
| ASSERT_EQ(val1,val2); | EXPECT_EQ(val1,val2); | val1 == val2 |
| ASSERT_NE(val1,val2); | EXPECT_NE(val1,val2); | val1 != val2 |
| ASSERT_LT(val1,val2); | EXPECT_LT(val1,val2); | val1 < val2 |
| ASSERT_LE(val1,val2); | EXPECT_LE(val1,val2); | val1 <= val2 |
| ASSERT_GT(val1,val2); | EXPECT_GT(val1,val2); | val1 > val2 |
| ASSERT_GE(val1,val2); | EXPECT_GE(val1,val2); | val1 >= val2 |

### String Comparison
For comparing two strings.
| Fatal assertion  | Nonfatal assertion | Verifies |
|---|---|---|
| ASSERT_STREQ(str1,str2); | EXPECT_STREQ(str1,_str_2); | same content |
| ASSERT_STRNE(str1,str2); | EXPECT_STRNE(str1,str2); | different content |
| ASSERT_STRCASEEQ(str1,str2); | EXPECT_STRCASEEQ(str1,str2); | same content and ignore case |
| ASSERT_STRCASENE(str1,str2); | EXPECT_STRCASENE(str1,str2); | different content and ignore case |

## mocking

When testing a units business logic, it's recommended to mock external dependencies, because in most cases their logic is already tested somewhere else. Because our unit depends on them, we have to make sure that the unit calls it, that is when mocking comes in useful.

In google mock we can mock by inheriting from the class to be mocked and wrapping functions in GoogleMock macros.

### example

Let's test a small sample unit (`client.uploadData(string data)`) that is calling an api (`api.post(string data)`).
Since we don't want to test the api and its response, we decide to mock the whole api call.

```c++
#include "gmock/gmock.h"  // bring in in Google Mock.

class ApiMock : public SomeApi {
 public:
  ...
  // Google Mock macro to mock a function called post with the return value void and a string argument called data
  MOCK_METHOD0(post, void(string data));
};

...

// GoogleTest Macro to create a test case named uploadData for the unit called client 
TEST_F(client, uploadData) {
  ApiMock mock; //bring in the mock object
  Client client = new Client(mock); //inject the mock into the unit
  string anyData = "any" //the sample data
  EXPECT_CALL(mock, post(anyData))
    .Times(1) //create a spy for the post call of the mock object, this macros creates a spy that validates, that the post()-method of the mock will be called with the argument anyData

  client.postData(anyData); //call the actual function
}
```

In this example, the whole logic of the client.uploadData() is tested without touching any api calls.

If our unit handles with broken api responses it's recommended mocking those as well. This can be done by telling our mock what to return on a call. We can then test if the function that called the api throws the correct exception (the unit needs to be inherited for that)

For cleaner code it's nice to use `using ::testing::Return;` at the top of the test if using the Return-Action.
```c++
#include "gmock/gmock.h" 
using ::testing::Return;

class ApiMock : public SomeApi {
 public:
  MOCK_METHOD0(get, HttpResponse(string url));
};
...
TEST_F(client, uploadData) {
ApiMock mock;
ClientMock client = new ClientMock(mock);
...
EXPECT_CALL(mock, get(::testing::_))//wildcard for any argument
  .WillOnce(Return(new HttpResponse(404))) // will return a 404 on call

// calls the fetchData with argument "anyUrl" and expects an NotFoundException
EXPECT_THROW(client.fetchData("anyUrl"), NotFoundException);
}
```
