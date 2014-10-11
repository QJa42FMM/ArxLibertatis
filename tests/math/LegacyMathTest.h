/*
 * Copyright 2011-2014 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_TESTS_GRAPHICS_LEGACYMATHTEST_H
#define ARX_TESTS_GRAPHICS_LEGACYMATHTEST_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "graphics/GraphicsUtility.h"

class LegacyMathTest : public CppUnit::TestFixture {
	
	CPPUNIT_TEST_SUITE(LegacyMathTest);	
	CPPUNIT_TEST(quaternionTests);
	CPPUNIT_TEST(quatMuliplyTest);
	//CPPUNIT_TEST(quatSlerpTest); ignore
	CPPUNIT_TEST(quatTransformVectorTest);
	CPPUNIT_TEST(quatMatrixConversionTest);
	CPPUNIT_TEST(vecMatrixConversionTest);
	CPPUNIT_TEST(angleTest);
	CPPUNIT_TEST(angleConversionTest);
	CPPUNIT_TEST(cameraRotationTest);
	CPPUNIT_TEST_SUITE_END();
	
public:
	LegacyMathTest()
		: CppUnit::TestFixture()
	{}
	
	void setUp();
	void tearDown();
	
	void quaternionTests();
	void quatMuliplyTest();
	void quatSlerpTest();
	void quatTransformVectorTest();
	void quatMatrixConversionTest();
	void vecMatrixConversionTest();
	void angleTest();
	void angleConversionTest();
	void cameraRotationTest();
};

#endif // ARX_TESTS_GRAPHICS_LEGACYMATHTEST_H
