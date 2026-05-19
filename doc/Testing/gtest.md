# GoogleTest and GoogleMock

GoogleTest is used to test C++ code, and GoogleMock is used to mock dependencies in those tests.
CrowNet currently uses version 1.10 because parts of the newer GoogleMock syntax are not yet compatible with this code base.

For more information about GoogleTest and GoogleMock, read the [official documentation](https://github.com/google/googletest/tree/master/docs).

The latest versions use a newer syntax. For the older syntax used here, see this [Chromium page](https://chromium.googlesource.com/external/github.com/google/googletest/+/refs/tags/release-1.8.0/googlemock/docs/ForDummies.md).

To run the GoogleTests in this project, navigate to `crownet` and execute them with the OMNeT++ container via `omnetpp exec make make-test MODE=debug`.

## Testing

To write a new GoogleTest class, create a file named `<unit_name>Test.cc` in the test directory.
In this project, tests often inherit from `BaseOppTest` to reuse OMNeT++-related helper functionality.

### Example

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

There are two assertion types: fatal (`ASSERT`) and non-fatal (`EXPECT`). A fatal assertion stops the current test when the expected condition is false. A non-fatal assertion continues the test and marks it as failed.

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

## Mocking

When testing a unit's business logic, it is recommended to mock external dependencies, because in most cases their own logic is already tested elsewhere. Since the unit depends on those components, we need to verify that the unit calls them correctly. This is where mocking is useful.

In GoogleMock, we create mocks by inheriting from the class to be mocked and wrapping functions with GoogleMock macros.

### example

Let's test a small sample unit (`client.uploadData(string data)`) that calls an API (`api.post(string data)`).
Since we do not want to test the API and its response here, we mock the API call.

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

In this example, the logic of `client.uploadData()` is tested without executing real API calls.

If the unit handles broken API responses, those cases should also be mocked. This can be done by configuring what the mock returns on a call. You can then test whether the function that calls the API throws the correct exception (the unit needs to be inherited for that).

For cleaner code, add `using ::testing::Return;` at the top of the test when using the `Return` action.
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
