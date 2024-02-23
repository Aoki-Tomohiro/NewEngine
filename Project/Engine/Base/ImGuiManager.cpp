#include "ImGuiManager.h"
#include "Engine/Externals/imgui/imgui_impl_win32.h"
#include "Engine/Externals/imgui/imgui_impl_dx12.h"

ImGuiManager* ImGuiManager::GetInstance() {
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::Initialize() {
	//Applicationのインスタンスを取得
	application_ = Application::GetInstance();

	//GraphicsCoreのインスタンスを取得
	graphicsCore_ = GraphicsCore::GetInstance();

	//SRVDescriptorHeapの作成
	srvDescriptorHeap_ = std::make_unique<DescriptorHeap>();
	srvDescriptorHeap_->Initialize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, true);

	//ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(application_->GetHwnd());
	ImGui_ImplDX12_Init(graphicsCore_->GetDevice(),
		2,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		srvDescriptorHeap_->GetDescriptorHeap(),
		srvDescriptorHeap_->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap_->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
}

void ImGuiManager::Begin() {
	//ImGuiにフレームの開始を伝える
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End() {
	//ImGuiの内部コマンドを生成する
	ImGui::Render();
}

void ImGuiManager::Draw() {
#ifdef _DEBUG

	//コマンドリストを取得
	CommandContext* commandContext = graphicsCore_->GetCommandContext();

	//描画用のDescriptorHeapの設定
	commandContext->SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, srvDescriptorHeap_->GetDescriptorHeap());

	//実際にcommandListのImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandContext->GetCommandList());

#endif
}

void ImGuiManager::ShutDown() {
	//解放処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	srvDescriptorHeap_.reset();
}