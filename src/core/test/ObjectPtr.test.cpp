#include <gtest/gtest.h>

import awen.core.object;
import awen.core.objectptr;

TEST(ObjectPtr, BasicUsage)
{
    auto object = std::make_unique<awn::core::Object>();
    awn::core::ObjectPtr<awn::core::Object> ptr(object.get());

    EXPECT_EQ(ptr->get_name(), "");
    ptr->set_name("Test Object");
    EXPECT_EQ(ptr->get_name(), "Test Object");

    // Destroy the object and check that the pointer becomes null.
    object.reset();
    EXPECT_EQ(ptr, nullptr);
}

TEST(ObjectPtr, Comparison)
{
    auto object1 = std::make_unique<awn::core::Object>();
    auto object2 = std::make_unique<awn::core::Object>();

    awn::core::ObjectPtr<awn::core::Object> ptr1(object1.get());
    awn::core::ObjectPtr<awn::core::Object> ptr2(object2);
    awn::core::ObjectPtr<awn::core::Object> ptr3(object1);

    EXPECT_EQ(ptr1, ptr3);
    EXPECT_NE(ptr1, ptr2);
    EXPECT_NE(ptr2, ptr3);

    // Destroy object1 and check that ptr1 and ptr3 become null.
    object1.reset();
    EXPECT_EQ(ptr1, nullptr);
    EXPECT_EQ(ptr3, nullptr);
}