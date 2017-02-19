#ifndef __SMARTPTR_H__
#define __SMARTPTR_H__

#include <cstddef>
#include <atomic>

namespace zuzuf
{

    class ref_count
    {
    public:
        inline ref_count() : counter(0)	{}

        inline ref_count &operator=(const ref_count &)	{	return *this;	}

        inline void retain()	{	++counter;	}
        inline size_t release()	{	return --counter;	}
        inline size_t get_ref_count() const	{	return counter;	}
    private:
        std::atomic<size_t> counter;
    };

    template<class T>
    class smartptr
    {
        template<class U> friend class smartptr;
    public:
        inline smartptr() : ptr(NULL)	{}
        inline smartptr(T *ptr) : ptr(ptr)
        {
            if (ptr)
                ptr->retain();
        }
        inline smartptr(const T *ptr) : ptr(const_cast<T*>(ptr))
        {
            if (this->ptr)
                this->ptr->retain();
        }
        inline smartptr(smartptr &ptr) : ptr(ptr.ptr)
        {
            if (this->ptr)
                this->ptr->retain();
        }
        inline smartptr(smartptr &&ptr) : ptr(ptr.ptr)
        {
            ptr.ptr = NULL;
        }
        inline smartptr(const smartptr &ptr) : ptr(const_cast<T*>(ptr.ptr))
        {
            if (this->ptr)
                this->ptr->retain();
        }
        template<class U>
        inline smartptr(const smartptr<U> &ptr) : ptr(dynamic_cast<T*>(const_cast<U*>(ptr.ptr)))
        {
            if (this->ptr)
                this->ptr->retain();
        }
        template<class U>
        inline smartptr(const U *ptr) : ptr(dynamic_cast<T*>(const_cast<U*>(ptr)))
        {
            if (this->ptr)
                this->ptr->retain();
        }
        inline ~smartptr()
        {
            if (ptr)
                clear(ptr);
        }

        inline smartptr &operator=(const smartptr &ptr)
        {
            if (this->ptr == ptr.ptr)
                return *this;
            if (!ptr.ptr)
            {
                if (this->ptr)
                    clear(this->ptr);
                this->ptr = NULL;
                return *this;
            }

            T *old = this->ptr;
            this->ptr = const_cast<T*>(ptr.ptr);
            if (this->ptr)
                this->ptr->retain();
            if (old)
                clear(old);
            return *this;
        }

        inline smartptr &operator=(smartptr &&ptr)
        {
            if (!ptr.ptr)
            {
                if (this->ptr)
                    clear(this->ptr);
                this->ptr = NULL;
                return *this;
            }

            if (this->ptr)
                clear(this->ptr);
            this->ptr = const_cast<T*>(ptr.ptr);
            ptr.ptr = NULL;
            return *this;
        }

        inline smartptr &operator=(const T *ptr)
        {
            if (this->ptr == ptr)
                return *this;

            if (!ptr)
            {
                if (this->ptr)
                    clear(this->ptr);
                this->ptr = NULL;
                return *this;
            }

            T *old = this->ptr;
            this->ptr = const_cast<T*>(ptr);
            if (this->ptr)
                this->ptr->retain();
            if (old)
                clear(old);
            return *this;
        }

        template<class U>
        inline smartptr &operator=(const U *ptr)
        {
            return operator=(dynamic_cast<const T*>(ptr));
        }

        inline T *operator->()	{	return ptr;	}
        inline const T *operator->() const	{	return ptr;	}
        inline T &operator*()	{	return *ptr;	}
        inline const T &operator*() const	{	return *ptr;	}

        inline operator bool() const	{	return ptr != NULL;	}

        template<class U>
        inline U *as()	{	return dynamic_cast<U*>(ptr);	}

        template<class U>
        inline const U *as() const	{	return dynamic_cast<const U*>(ptr);	}

        inline T * const &weak() const	{	return ptr;	}
        inline T * &weak()	{	return ptr;	}

        operator T*() const {   return ptr; }

    private:
        inline void clear(T *p) const
        {
            if (p->release() == 0)
                delete p;
        }
    private:
        T *ptr;
    };
}

namespace std
{
    template<typename T> struct hash;

    template<typename T>
    struct hash<zuzuf::smartptr<T> > : public hash<T*>
    {
        inline size_t operator()(const zuzuf::smartptr<T> &v) const
        {
            return hash<T*>::operator()(v.weak());
        }
    };
}

#endif
