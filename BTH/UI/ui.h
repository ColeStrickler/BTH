#pragma once
#include "../Dependencies/imgui/imgui.h"
#include "../Dependencies/imgui/imgui_impl_glfw.h"
#include "../Dependencies/imgui/imgui_impl_opengl3.h"
#include <vector>
#include <functional>
#include <unordered_map>
#include <string>
#include <any>
#include <iostream>

template <typename T>
struct UI_ReturnHolder
{
	int m_ID;
	T m_ReturnData;
};

struct ComponentFunctionCall
{
	std::string m_ComponentName;
	std::function<void()> m_Func;
};



class UI
{
public:
	UI(int WindowWidth, int WindowHeight, int LeftWidth = 150, 
		int RightWidth = 150, int BottomHeight = 200, int TopHeight = 200);
	~UI();

	void Init(GLFWwindow* Window);
	void DrawUI() const;
	void Start() const;
	void End();
	void Cleanup();



	/*
		When we call this we use the template to specify the return type
	*/
	template<typename T>
	T GetComponentReturnVal(std::string name)
	{
		if (m_ComponentReturnValues.count(name) == 0)
		{
			std::cout << "[UI_FRAMEWORK 0.1] -> Warning Component " << name <<
				"does not exist. Returning NULL" << std::endl;
			return (T)NULL;
		}
		return std::any_cast<T>(m_ComponentReturnValues[name]);
	}


	



private:
	void DisplayComponentsRight() const;
	void DisplayComponentsLeft() const;
	void DisplayComponentsTop() const;
	void DisplayComponentsBottom() const;



private:
	std::vector<ComponentFunctionCall> m_ComponentsRight;
	std::vector<ComponentFunctionCall> m_ComponentsLeft;
	std::vector<ComponentFunctionCall> m_ComponentsTop;
	std::vector<ComponentFunctionCall> m_ComponentsBottom;
	std::unordered_map<std::string, std::any> m_ComponentReturnValues;
	int m_WindowWidth;
	int m_WindowHeight;
	int m_LeftWidth;
	int m_RightWidth;
	int m_BottomHeight;
	int m_TopHeight;
	GLFWwindow* m_Window;







/*
	UI Component add functions	
*/
public:
	/*
		NOTE: We cannot add component Functions that return void
		Because we cannot store void in the unordered_map m_ComponentReturnValues.
	*/
	template <typename Func, typename... Args>
	void AddComponentRight(std::string name, Func&& func, Args&&... args) {

		if (m_ComponentReturnValues.count(name) > 0)
		{
			std::cout << "[UI_FRAMEWORK 0.1] -> " << "Component name" << name << "cannot be used twice.\
			 Please utilize unique names for your UI components." << std::endl;
			__debugbreak();
		}

		ComponentFunctionCall cfc;
		//auto call = [func, args...](){std::invoke(func, args...); };
		std::function<void()> functionCall = [=]() {
			m_ComponentReturnValues[name] = func(args...); // Call the function with the provided arguments
		};
		cfc.m_ComponentName = name;
		cfc.m_Func = functionCall;
		m_ComponentsRight.push_back(cfc);
	}

	template <typename Func, typename... Args>
	void AddComponentLeft(std::string name, Func&& func, Args&&... args) {
		if (m_ComponentReturnValues.count(name) > 0)
		{
			std::cout << "[UI_FRAMEWORK 0.1] -> " << "Component name" << name << "cannot be used twice.\
			 Please utilize unique names for your UI components." << std::endl;
			__debugbreak();
		}
		ComponentFunctionCall cfc;
		//auto call = [func, args...](){std::invoke(func, args...); };
		std::function<void()> functionCall = [=]() {
			m_ComponentReturnValues[name] = func(args...); // Call the function with the provided arguments
		};
		cfc.m_ComponentName = name;
		cfc.m_Func = functionCall;
		m_ComponentsLeft.push_back(cfc);
	}

	template <typename Func, typename... Args>
	void AddComponentTop(std::string name, Func&& func, Args&&... args) {
		if (m_ComponentReturnValues.count(name) > 0)
		{
			std::cout << "[UI_FRAMEWORK 0.1] -> " << "Component name" << name << "cannot be used twice.\
			 Please utilize unique names for your UI components." << std::endl;
			__debugbreak();
		}
		ComponentFunctionCall cfc;
		//auto call = [func, args...](){std::invoke(func, args...); };
		std::function<void()> functionCall = [=]() {
			m_ComponentReturnValues[name] = func(args...); // Call the function with the provided arguments
		};
		cfc.m_ComponentName = name;
		cfc.m_Func = functionCall;
		m_ComponentsTop.push_back(cfc);
	}

	template <typename Func, typename... Args>
	void AddComponentBottom(std::string name, Func&& func, Args&&... args) {
		if (m_ComponentReturnValues.count(name) > 0)
		{
			std::cout << "[UI_FRAMEWORK 0.1] -> " << "Component name" << name << "cannot be used twice.\
			 Please utilize unique names for your UI components." << std::endl;
			__debugbreak();
		}
		ComponentFunctionCall cfc;
		//auto call = [func, args...](){std::invoke(func, args...); };
		std::function<void()> functionCall = [=]() {
			m_ComponentReturnValues[name] = func(args...); // Call the function with the provided arguments
		};
		cfc.m_ComponentName = name;
		cfc.m_Func = functionCall;
		m_ComponentsBottom.push_back(cfc);
	}





};

