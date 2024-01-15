#include "Renderer.h"
#include "Scene.h"

#include <glm/gtc/type_ptr.hpp>
#include <Walnut/Application.h>
#include <Walnut/EntryPoint.h>
#include <Walnut/Image.h>
#include <Walnut/Timer.h>

class ExampleLayer : public Walnut::Layer
{
public:

	ExampleLayer() : m_camera(45.0f, 0.01f, 1000.0f)
	{
		Material material;
		Sphere sphere;

		// Pink Sphere
		material.albedo = glm::vec3{ 1.0f, 0.0f, 0.8f };
		material.roughness = 1.0f;
		material.metallic = 0.0f;
		material.emissiveColor = glm::vec3{ 0.0f, 0.0f , 0.0f };
		material.emissiveIntensity = 0.0f;
		material.isTransparent = false;
		material.eta = 1.0f;
		m_scene.materials.emplace_back(std::move(material));
		sphere.materialIndex = m_scene.materials.size() - 1;
		sphere.center = glm::vec3{ -0.5f, 0.0f, -1.0f };
		sphere.radius = 0.5f;
		m_scene.spheres.emplace_back(std::move(sphere));
		
		// Mirror Sphere
		material.albedo = glm::vec3{ 1.0f, 1.0f, 1.0f };
		material.roughness = 0.0f;
		material.metallic = 1.0f;
		material.emissiveColor = glm::vec3{ 0.0f, 0.0f , 0.0f };
		material.emissiveIntensity = 0.0f;
		material.isTransparent = false;
		material.eta = 1.0f;
		m_scene.materials.emplace_back(std::move(material));
		sphere.materialIndex = m_scene.materials.size() - 1;
		sphere.center = glm::vec3{ 0.5f, 0.0f, -1.0f };
		sphere.radius = 0.5f;
		m_scene.spheres.emplace_back(std::move(sphere));

		// Glass Sphere
		material.albedo = glm::vec3{ 1.0f, 1.0f, 1.0f };
		material.roughness = 0.0f;
		material.metallic = 0.0f;
		material.emissiveColor = glm::vec3{ 0.0f, 0.0f , 0.0f };
		material.emissiveIntensity = 0.0f;
		material.isTransparent = true;
		material.eta = 1.52f; // Water
		m_scene.materials.emplace_back(std::move(material));
		sphere.materialIndex = m_scene.materials.size() - 1;
		sphere.center = glm::vec3{ 0.0f, -0.2f, 0.2f };
		sphere.radius = 0.1f;
		m_scene.spheres.emplace_back(std::move(sphere));

		// Ground
		material.albedo = glm::vec3{ 0.5f, 0.5f, 0.5f };
		material.roughness = 1.0f;
		material.metallic = 0.0f;
		material.emissiveColor = glm::vec3{ 0.0f, 0.0f , 0.0f };
		material.emissiveIntensity = 0.0f;
		material.isTransparent = false;
		material.eta = 1.0f;
		m_scene.materials.emplace_back(std::move(material));
		sphere.materialIndex = m_scene.materials.size() - 1;
		sphere.center = glm::vec3{ 0.0f, -100.5f, -1.0f };
		sphere.radius = 100.0f;
		m_scene.spheres.emplace_back(std::move(sphere));
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_camera.OnUpdate(ts))
		{
			m_renderer.StopAccumulate();
		}
		else
		{
			m_renderer.StartAccumulate();
		}
	}

	virtual void OnUIRender() override
	{
		bool needReset = false;
		auto CheckReset = [&needReset](bool reset)
		{
			if (reset)
			{
				needReset = true;
			}
		};

		// Settings
		ImGui::Begin("Setting");
		ImGui::Text("Last Frame: %.3fms", m_lastFrameTime);
		ImGui::Separator();
		CheckReset(ImGui::DragInt("Bounces", &m_renderer.GetBounces(), 0.05f, 0, 32));
		ImGui::End();

		// Object List
		ImGui::Begin("Objects");
		for (size_t i = 0; i < m_scene.spheres.size(); ++i)
		{
			ImGui::PushID(i);
			Sphere &sphere = m_scene.spheres[i];
			CheckReset(ImGui::DragFloat3("Position", glm::value_ptr(sphere.center), 0.01f));
			CheckReset(ImGui::DragFloat("Radius", &sphere.radius, 0.01f, 0.0f, 100.0f));
			CheckReset(ImGui::DragInt("Material", &sphere.materialIndex, 1, 0, (int)m_scene.materials.size() - 1));
			ImGui::Separator();
			ImGui::PopID();
		}
		ImGui::End();

		// Material List
		ImGui::Begin("Materials");
		for (size_t i = 0; i < m_scene.materials.size(); ++i)
		{
			Material &material = m_scene.materials[i];
			ImGui::PushID(i);
			CheckReset(ImGui::ColorEdit3("Albedo", glm::value_ptr(material.albedo)));
			CheckReset(ImGui::DragFloat("Roughness", &material.roughness, 0.01f, 0.0f, 1.0f));
			CheckReset(ImGui::DragFloat("Metallic", &material.metallic, 0.01f, 0.0f, 1.0f));
			CheckReset(ImGui::ColorEdit3("Emissive Color", glm::value_ptr(material.emissiveColor)));
			CheckReset(ImGui::DragFloat("Emissive Intensity", &material.emissiveIntensity, 0.01f, 0.0f, 100.0f));
			ImGui::Separator();
			ImGui::PopID();
		}
		ImGui::End();

		// Scene View
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Scene");
		m_viewportWidth = ImGui::GetContentRegionAvail().x;
		m_viewportHeight = ImGui::GetContentRegionAvail().y;
		if (auto pImage = m_renderer.GetFinalImage(); pImage)
		{
			ImGui::Image(pImage->GetDescriptorSet(),
				ImVec2{ (float)pImage->GetWidth(), (float)pImage->GetHeight() },
				ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });
		}
		ImGui::End();
		ImGui::PopStyleVar();

		if (needReset)
		{
			m_renderer.StopAccumulate();
		}

		Render();
	}

	void Render()
	{
		Walnut::Timer timer;

		m_camera.OnResize(m_viewportWidth, m_viewportHeight);

		m_renderer.OnResize(m_viewportWidth, m_viewportHeight);
		m_renderer.Render(m_scene, m_camera);

		m_lastFrameTime = timer.ElapsedMillis();
	}

private:
	float m_lastFrameTime = 0.0f;

	uint32_t m_viewportWidth = 0;
	uint32_t m_viewportHeight = 0;

	Camera m_camera;
	Renderer m_renderer;
	Scene m_scene;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}
