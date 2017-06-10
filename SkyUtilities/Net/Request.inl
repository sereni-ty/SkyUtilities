template<class ProtocolContextType>
Request::Ptr Request::Create(uint32_t pre_set_id)
{
  Ptr new_request = std::make_shared<Request>(pre_set_id);

  if (new_request == nullptr)
  {
    Plugin::Log(LOGL_INFO, "Request: Unable to allocate memory for request.");
    throw std::bad_exception();
  }

  new_request->proto_ctx = std::make_shared<ProtocolContextType>();

  if (new_request->proto_ctx == nullptr)
  {
    Plugin::Log(LOGL_INFO, "Request: Unable to allocate memory for protocol contextual data.");
    throw std::bad_exception();
  }

  new_request->proto_ctx->SetOwner(new_request);

  Plugin::Log(LOGL_VERBOSE, "Request (id: %d): Request created", new_request->id);

  return new_request;
}

template< class ProtocolContextType >
typename ProtocolContextType::Ptr Request::GetProtocolContext()
{
  return std::dynamic_pointer_cast<ProtocolContextType>(proto_ctx);
}